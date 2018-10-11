/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "shaderpack_loading.hpp"
#include "../folder_accessor.hpp"
#include "../loading_utils.hpp"
#include "../zip_folder_accessor.hpp"
#include "../regular_folder_accessor.hpp"
#include "../json_utils.hpp"
#include "json_interop.hpp"
#include <ftl/atomic_counter.h>
#include "shaderpack_validator.hpp"
#include "render_graph_builder.hpp"
#include <shaderc/shaderc.hpp>

namespace nova {
    folder_accessor_base *get_shaderpack_accessor(const fs::path &shaderpack_name);

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, shaderpack_resources_data &output);
    void load_passes_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, std::vector<render_pass_data> &output);
    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, std::vector<pipeline_data> &output);
    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &pipeline_path, uint32_t out_idx, std::vector<pipeline_data> &output);
    void load_material_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, std::vector<material_data> &output);
    void load_single_material(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &material_path, uint32_t out_idx, std::vector<material_data> &output);

    std::vector<uint32_t> load_shader_file(const fs::path& filename, folder_accessor_base* folder_access, shaderc_shader_kind stage, const std::vector<std::string>& defines);

    bool loading_failed = false;

    std::optional<shaderpack_data> load_shaderpack_data(const fs::path &shaderpack_name, ftl::TaskScheduler &task_scheduler) {
        loading_failed = false;
        folder_accessor_base *folder_access = get_shaderpack_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem

        ftl::AtomicCounter loading_tasks_remaining(&task_scheduler);

        shaderpack_data data = {};

        // Load resource definitions
        shaderpack_resources_data loaded_resources = {};
        task_scheduler.AddTask(&loading_tasks_remaining, load_dynamic_resources_file, folder_access, data.resources);

        // Load pass definitions
        std::vector<render_pass_data> loaded_passes;
        task_scheduler.AddTask(&loading_tasks_remaining, load_passes_file, folder_access, data.passes);

        // Load pipeline definitions
        std::vector<pipeline_data> loaded_pipelines;
        task_scheduler.AddTask(&loading_tasks_remaining, load_pipeline_files, folder_access, data.pipelines);

        // Load materials
        std::vector<material_data> loaded_materials;
        task_scheduler.AddTask(&loading_tasks_remaining, load_material_files, folder_access, data.materials);

        task_scheduler.WaitForCounter(&loading_tasks_remaining, 0);

        delete folder_access;

        if(loading_failed) {
            return {};

        } else {
            return std::make_optional(data);
        }
    }

    folder_accessor_base *get_shaderpack_accessor(const fs::path &shaderpack_name) {
        folder_accessor_base *folder_access = nullptr;
        fs::path path_to_shaderpack = shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            folder_access = new zip_folder_accessor(path_to_shaderpack);

        } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            folder_access = new regular_folder_accessor(path_to_shaderpack);

        } else {
            path_to_shaderpack = shaderpack_name;

            if(is_zip_folder(path_to_shaderpack)) {
                // zip folder in the resourcepacks folder
                path_to_shaderpack.replace_extension(".zip");
                folder_access = new zip_folder_accessor(path_to_shaderpack);

            } else if(fs::exists(path_to_shaderpack)) {
                folder_access = new regular_folder_accessor(path_to_shaderpack);
            }
        }

        if(folder_access == nullptr) {
            throw resource_not_found_exception(shaderpack_name.string());
        }

        return folder_access;
    }

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, shaderpack_resources_data &output) {
        std::string resources_string = folder_access->read_text_file("resources.json");
        try {
            auto json_resources = nlohmann::json::parse(resources_string.c_str());
            const validation_report report = validate_shaderpack_resources_data(json_resources);
            print(report);
            if(!report.errors.empty()) {
                loading_failed = true;
                return;
            }

            auto resources = json_resources.get<shaderpack_resources_data>();
            output.samplers = std::move(resources.samplers);
            output.textures = std::move(resources.textures);

        } catch(resource_not_found_exception &) {
            // No resources defined.. I guess they think they don't need any?
            NOVA_LOG(WARN) << "No resources file found for shaderpack at " << folder_access->get_root();
            loading_failed = true;

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's resources.json: " << err.what();
            loading_failed = true;

        } catch(validation_failure_exception &err) {
            NOVA_LOG(ERROR) << "Could not validate resources.json: " << err.what();
            loading_failed = true;
        }
    }

    void load_passes_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, std::vector<render_pass_data> &output) {
        const auto passes_bytes = folder_access->read_text_file("passes.json");
        try {
            auto json_passes = nlohmann::json::parse(passes_bytes);
            auto passes = json_passes.get<std::vector<render_pass_data>>();

            std::unordered_map<std::string, render_pass_data> passes_by_name;
            passes_by_name.reserve(passes.size());
            for(const auto &pass : passes) {
                passes_by_name[pass.name] = pass;
            }

            const auto ordered_pass_names = order_passes(passes_by_name);
            passes.clear();
            for(const auto named_pass : ordered_pass_names) {
                passes.push_back(passes_by_name.at(named_pass));
            }

            output = std::move(passes);

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's passes.json: " << err.what();
            loading_failed = true;
        }

        // Don't check for a resources_not_found exception because a shaderpack _needs_ a passes.json and if the
        // shaderpack doesn't provide one then it can't be loaded, so we'll catch that exception later on
    }

    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, std::vector<pipeline_data> &output) {
        std::vector<fs::path> potential_pipeline_files;
        try {
            potential_pipeline_files = folder_access->get_all_items_in_folder("materials");
        } catch(filesystem_exception &exception) {
            NOVA_LOG(ERROR) << "Materials folder does not exist: " << exception.what();
            loading_failed = true;
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.resize(potential_pipeline_files.size());
        uint32_t num_pipelines = 0;
        ftl::AtomicCounter pipeline_load_tasks_remaining(task_scheduler);

        for(const fs::path &potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                task_scheduler->AddTask(&pipeline_load_tasks_remaining, load_single_pipeline, folder_access, potential_file, num_pipelines, output);
                num_pipelines++;
            }
        }

        task_scheduler->WaitForCounter(&pipeline_load_tasks_remaining, 0);
    }

    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &pipeline_path, uint32_t out_idx, std::vector<pipeline_data> &output) {
        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);
        try {
            auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
            const validation_report report = validate_graphics_pipeline(json_pipeline);
            print(report);
            if(!report.errors.empty()) {
                loading_failed = true;
                return;
            }

            pipeline_data new_pipeline = json_pipeline.get<pipeline_data>();
            new_pipeline.vertex_shader.source = load_shader_file(new_pipeline.vertex_shader.filename, folder_access, shaderc_vertex_shader, new_pipeline.defines);

            output[out_idx] = new_pipeline;

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse pipeline file " << pipeline_path.string() << ": " << err.what();
            loading_failed = true;

            //} catch(nlohmann::json::type_error& err) {
            //    NOVA_LOG(ERROR) << "Could not parse pipeline file " << pipeline_path.string() << ": " << err.what();

        } catch(validation_failure_exception &err) {
            NOVA_LOG(ERROR) << "Could not validate pipeline file " << pipeline_path.string() << ": " << err.what();
            loading_failed = true;
        }
    }

    std::vector<uint32_t> load_shader_file(const fs::path& filename, folder_accessor_base* folder_access, const shaderc_shader_kind stage, const std::vector<std::string>& defines) {
        static std::unordered_map<shaderc_shader_kind, std::vector<fs::path>> extensions_by_shader_stage = {
            {shaderc_vertex_shader,  {
                ".vert.spirv",
                ".vsh.spirv",
                ".vertex.spirv",

                ".vert",
                ".vsh",

                ".vertex",

                ".vert.hlsl",
                ".vsh.hlsl",
                ".vertex.hlsl",
            }},
            {shaderc_fragment_shader, {
                ".frag.spirv",
                ".fsh.spirv",
                ".fragment.spirv",

                ".frag",
                ".fsh",

                ".fragment",

                ".frag.hlsl",
                ".fsh.hlsl",
                ".fragment.hlsl",
            }},

            {shaderc_geometry_shader, {
                ".geom.spirv",
                ".geo.spirv",
                ".geometry.spirv",

                ".geom",
                ".geo",

                ".geometry",

                ".geom.hlsl",
                ".geo.hlsl",
                ".geometry.hlsl",
            }},
            {shaderc_tess_evaluation_shader, {
                ".tese.spirv",
                ".tse.spirv",
                ".tess_eval.spirv",

                ".tese",
                ".tse",

                ".tess_eval",

                ".tese.hlsl",
                ".tse.hlsl",
                ".tess_eval.hlsl",
            }},
            {shaderc_tess_control_shader, {
                ".tesc.spirv",
                ".tsc.spirv",
                ".tess_control.spirv",

                ".tesc",
                ".tsc",

                ".tess_control",

                ".tesc.hlsl",
                ".tsc.hlsl",
                ".tess_control.hlsl",
            }}
        };

        std::vector<fs::path> extensions_for_current_stage = extensions_by_shader_stage.at(stage);

        for(const fs::path& extension : extensions_for_current_stage) {
            fs::path full_filename = filename;
            full_filename.replace_extension(extension);

            if(!folder_access->does_resource_exist(full_filename)) {
                continue;
            }

            std::unique_ptr<shader_includer> includer = std::make_unique<shader_includer>(*folder_access);
            shaderc::Compiler compiler;
            shaderc::CompileOptions options;
            options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
            options.SetWarningsAsErrors();
            options.SetGenerateDebugInfo();

            // TODO: Figure out how to handle shader options
            for(const std::string& define : defines) {
                options.AddMacroDefinition(define);
            }

            options.SetIncluder(std::move(includer));

            // Check the extension to know what kind of shader file the user has provided. SPIR-V files can be loaded 
            // as-is, but GLSL, GLSL ES, and HLSL files need to be transpiled to SPIR-V
            if(extension.string().find(".spirv") != std::string::npos) {
                // SPIR-V file!
                return folder_access->read_spirv_file(full_filename);

            } else if(extension.string().find(".hlsl") != std::string::npos) {
                // HLSL file! Let's convert
                options.SetSourceLanguage(shaderc_source_language_hlsl);

                std::string hlsl_source = folder_access->read_text_file(full_filename);
                shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(hlsl_source, stage, full_filename.string().data(), options);

                if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
                    throw shader_compilation_failed(result.GetErrorMessage());
                }

                return { result.cbegin(), result.cend() };
            }
        }
    }

    void load_material_files(ftl::TaskScheduler* task_scheduler, folder_accessor_base* folder_access, std::vector<material_data>& output) {
        std::vector<fs::path> potential_material_files;
        try {
            potential_material_files = folder_access->get_all_items_in_folder("materials");

        } catch(filesystem_exception &exception) {
            NOVA_LOG(ERROR) << "Materials folder does not exist: " << exception.what();
            loading_failed = true;
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.resize(potential_material_files.size());
        ftl::AtomicCounter material_load_tasks_remaining(task_scheduler);

        uint32_t num_materials = 0;

        for(const fs::path &potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                task_scheduler->AddTask(&material_load_tasks_remaining, load_single_material, folder_access, potential_file, num_materials, output);
                num_materials++;
            }
        }

        task_scheduler->WaitForCounter(&material_load_tasks_remaining, 0);
    }

    void load_single_material(ftl::TaskScheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &material_path, const uint32_t out_idx, std::vector<material_data> &output) {
        const std::string material_text = folder_access->read_text_file(material_path);
        try {
            auto json_material = nlohmann::json::parse(material_text);
            const auto report = validate_material(json_material);
            print(report);
            if(!report.errors.empty()) {
                // There were errors, this material can't be loaded
                loading_failed = true;
                return;
            }

            auto material = json_material.get<material_data>();
            material.name = material_path.stem().string();
            output[out_idx] = material;

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse material file " << material_path.string() << ": " << err.what();
            loading_failed = true;
        }
    }
}  // namespace nova
