set(LIBURL_PARSER_SOURCES
    url-parser/url_parser.cpp
)

add_library(url-parser OBJECT ${LIBURL_PARSER_SOURCES})
set_target_properties(url-parser PROPERTIES POSITION_INDEPENDENT_CODE ON)
