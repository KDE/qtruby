project(Korundum)

SET(CUSTOM_RUBY_SITE_ARCH_DIR ${RUBY_SITEARCH_DIR} CACHE DIR "custom installation directory for ruby binary extension" )
SET(CUSTOM_RUBY_SITE_LIB_DIR ${RUBY_SITELIB_DIR} CACHE DIR "custom installation directory for ruby extension" )

# compute an overall version number which can be compared at once
MATH(EXPR RUBY_VERSION_NUMBER "${RUBY_VERSION_MAJOR}*10000 + ${RUBY_VERSION_MINOR}*100 + ${RUBY_VERSION_PATCH}")

add_subdirectory( examples )
add_subdirectory( src )
add_subdirectory( tools )
add_subdirectory( modules )
