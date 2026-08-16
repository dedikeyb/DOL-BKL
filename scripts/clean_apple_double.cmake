# Deletes AppleDouble (._*) junk files recursively under CLEAN_DIR.
# Used as a POST_BUILD step: files with extended attributes on exFAT
# volumes get a ._ sidecar created by macOS; the bundle should not ship them.
if(NOT DEFINED CLEAN_DIR)
    message(FATAL_ERROR "CLEAN_DIR not set")
endif()

# Normalize: the caller may pass a path containing ".." components,
# which file(GLOB_RECURSE) cannot match.
file(REAL_PATH "${CLEAN_DIR}" clean_dir)

file(GLOB_RECURSE apple_double_files "${clean_dir}/._*")
foreach(junk IN LISTS apple_double_files)
    file(REMOVE "${junk}")
endforeach()
