/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_LOADING_HPP
#define NOVA_RENDERER_SHADERPACK_LOADING_HPP

#include <optional>
#include "shaderpack_data.hpp"

#if _WIN32
    #if _MSC_VER <= 1915
        #include <experimental/filesystem>
        namespace fs = std::experimental::filesystem;
    #else
        #include <filesystem>
        namespace fs = std::filesystem;
    #endif
#else
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

namespace nova {
    namespace ttl {
        class task_scheduler;
    }

    NOVA_EXCEPTION(shader_compilation_failed);
    NOVA_EXCEPTION(shader_reflection_failed);
    NOVA_EXCEPTION(shader_layout_creation_failed);
	NOVA_EXCEPTION(pipeline_load_failed);
	NOVA_EXCEPTION(material_load_failed);

    /*!
     * \brief Loads all the data for a single shaderpack
     *
     * This function reads the shaderpack data from disk (either a folder od a zip file) and performs basic validation
     * to ensure both that the data is well-formatted JSON, but also to ensure that the data has all the fields that
     * Nova requires, e.g. a material must have at least one pass, a texture must have a width and a height, etc. All
     * generated warnings and errors are printed to the Nova logger
     *
     * If the shaderpack can't be loaded, an empty optional is returned
     *
     * Note: This function is NOT thread-safe. It should only be called for a single thread at a time
     *
     * \param shaderpack_name The name of the shaderpack to load
     * \param task_scheduler The task scheduler to use when kicking off tasks to process all of the shaderpack data
     * \return The shaderpack, if it can be loaded, or an empty optional if it cannot
     */
    std::optional<shaderpack_data> load_shaderpack_data(const fs::path &shaderpack_name, ttl::task_scheduler &task_scheduler);
}  // namespace nova

#endif  // NOVA_RENDERER_SHADERPACK_LOADING_HPP
