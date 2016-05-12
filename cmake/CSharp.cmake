function(cs_add_assembly TARGET)
  set(CURR_LIST)
  set(FLAGS)
  set(SOURCES)
  set(DEPENDS)
  set(LIBRARIES)
  foreach(ARG ${ARGN})
    if(ARG STREQUAL "FLAGS")
      set(CURR_LIST "FLAGS")
    elseif(ARG STREQUAL "SOURCES")
      set(CURR_LIST "SOURCES")
    elseif(ARG STREQUAL "DEPENDS")
      set(CURR_LIST "DEPENDS")
    elseif(ARG STREQUAL "LIBRARIES")
      set(CURR_LIST "LIBRARIES")
    elseif(ARG STREQUAL "FOLDER")
      set(CURR_LIST "FOLDER")
    else()
      if(NOT CURR_LIST)
        message(FATAL_ERROR "Error in cs_add_assembly(...) arguments")
      endif()
      list(APPEND "${CURR_LIST}" ${ARG})
      if(ARG STREQUAL "FOLDER")
        # Only a single item is allowed in this list
        unset(CURR_LIST)
      endif()
    endif()
  endforeach()

  set(TARGET_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TARGET}")
  set(ASSEMBLY_FILE "${TARGET_FILE}")
  if(CYGWIN)
    execute_process(COMMAND cygpath -w -a "${ASSEMBLY_FILE}" OUTPUT_VARIABLE ASSEMBLY_FILE OUTPUT_STRIP_TRAILING_WHITESPACE)
  elseif(HOST_WINDOWS)
    file(TO_NATIVE_PATH "${ASSEMBLY_FILE}" ASSEMBLY_FILE)
  endif()
  set(OPTS -out:${ASSEMBLY_FILE} ${FLAGS})
  if("${TARGET}" MATCHES ".*\\.dll")
    set(OPTS ${OPTS} -target:library)
  endif()
  foreach(LIB ${LIBRARIES})
    if(TARGET ${LIB})
      get_target_property(F ${LIB} "ASSEMBLY_FILE")
      if(F)
        set(OPTS ${OPTS} "-r:${F}")
        set(DEPENDS ${DEPENDS} ${LIB})
      else()
        set(OPTS ${OPTS} "-r:${LIB}")
      endif()
    else()
      set(OPTS ${OPTS} "-r:${LIB}")
    endif()
  endforeach()

  add_custom_command(OUTPUT ${TARGET_FILE}
    COMMAND ${MCS_WRAPPER} ${OPTS} ${SOURCES}
    DEPENDS mcs-wrapper ${DEPENDS} ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(${TARGET} DEPENDS ${TARGET_FILE} SOURCES ${SOURCES})
  set_target_properties(${TARGET} PROPERTIES "ASSEMBLY_FILE" ${ASSEMBLY_FILE})
  if(FOLDER)
    set_target_properties(${TARGET} PROPERTIES FOLDER ${FOLDER})
  endif()
endfunction()
