# - Find Nuget
#
# This module guarantees that nuget and our required custom extensions are installed.
# After installation, the script NugetCore.cmake (located inside the CoAppCMake package) is
# executed. That's where the actual dependency mechanism is implemented.
#
# Optional environment variables:
# NUGET_UNPACK_DIR - Directory where the dependencies will be unpacked. Defaults to %TMP%.
# 
#=============================================================================
# Copyright 2015 SiVIEP
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

IF(_NUGET_INCLUDED)
	RETURN()
ENDIF()

SET (_NUGET_INCLUDED 1)

SET (NUGET_SERVER "http://taruma/api/odata/" CACHE INTERNAL "Online Nuget repository.")
SET (NUGET_DEBUG  FALSE                      CACHE INTERNAL "Print debug messages?")

FUNCTION (_nt_message P_MODE)
	SET (_OPTIONS FATAL_ERROR WARNING STATUS)
	
	LIST (FIND _OPTIONS "${P_MODE}" _INDEX)

	IF ("POWER_WARNING" STREQUAL "${P_MODE}")
		MESSAGE(STATUS "")
		MESSAGE(STATUS "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv")
		MESSAGE("")
		MESSAGE(WARNING "${ARGN}")
		MESSAGE(STATUS "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^")
	ELSEIF(${_INDEX} GREATER -1 OR NUGET_DEBUG)
		MESSAGE("${P_MODE}" "${ARGN}")
	ENDIF()
ENDFUNCTION ()

FUNCTION (_nt_search_coapp_nuget_executable P_RETVAR P_UNPACK_DIR P_VERSION)
    SET (_PREFIX ${P_UNPACK_DIR}/CoAppCMake.)    

    IF (P_VERSION)
        SET (_TEMP ${_PREFIX}${P_VERSION}/nuget.exe)
        
		MESSAGE(STATUS ${_TEMP})
		
        IF (EXISTS ${_TEMP})
			SET (${P_RETVAR} ${_TEMP} PARENT_SCOPE)
        ENDIF()
        
        RETURN()
    ENDIF()
    
    # Search for the highest version
    SET (_MAX_VERSION 0.0)
    
    STRING (LENGTH "${_PREFIX}" _PREFIX_SIZE)
    
    FILE (GLOB _COAPP_PACKAGES ${_PREFIX}*)
    
	FOREACH (_PACKAGE ${_COAPP_PACKAGES})
        IF (EXISTS 	${_PACKAGE}/nuget.exe)
            STRING (SUBSTRING ${_PACKAGE} ${_PREFIX_SIZE} -1 _PACKAGE_VERSION)
            
            IF (${_PACKAGE_VERSION} VERSION_GREATER ${_MAX_VERSION})
                SET (_MAX_VERSION ${_PACKAGE_VERSION})
            ENDIF()
		ENDIF()
	ENDFOREACH()

    IF (NOT ${_MAX_VERSION} VERSION_EQUAL "0.0")
        SET (${P_RETVAR} ${_PREFIX}${_MAX_VERSION}/nuget.exe PARENT_SCOPE)
    ENDIF()
ENDFUNCTION()

FUNCTION (_nt_install_nuget P_RETVAR P_UNPACK_DIR P_VERSION)
	SET (_TEMP_NUGET ${P_UNPACK_DIR}/nuget.exe)
	
	## Download the standard nuget...
	IF (NOT EXISTS ${_TEMP_NUGET})
		_nt_message (STATUS "Downloading standard nuget.exe from http://www.nuget.org/nuget.exe ...")
	
		FILE(DOWNLOAD http://www.nuget.org/nuget.exe ${_TEMP_NUGET})
	ENDIF()
		
	IF (NOT EXISTS ${_TEMP_NUGET})
		_nt_message(FATAL_ERROR "Failed to download nuget.exe.")
	ENDIF()
	
	_nt_message (STATUS "Using the standard nuget.exe to download CoAppCMake package with our own extensions...")
	
    SET (ARGS install CoAppCMake -Source ${NUGET_SERVER})
    IF (P_VERSION)
        SET(ARGS ${ARGS} -Version ${P_VERSION})
    ENDIF()

	EXEC_PROGRAM(${_TEMP_NUGET} ${P_UNPACK_DIR} 
				ARGS ${ARGS}
				RETURN_VALUE _ERRORLEVEL)
	
	FILE(REMOVE ${_TEMP_NUGET})
	
	IF (NOT ${_ERRORLEVEL} EQUAL 0)
		_nt_message (FATAL_ERROR "Could not find nuget package CoAppCMake.")
	ENDIF ()
	
	_nt_search_coapp_nuget_executable (_NUGET_EXECUTABLE ${P_UNPACK_DIR} "${P_VERSION}")
	
	IF (EXISTS ${_NUGET_EXECUTABLE})
		_nt_message (STATUS "Installation successful. Using ${_NUGET_EXECUTABLE}. Let's roll!")
	ENDIF()
	
	SET (${P_RETVAR} ${_NUGET_EXECUTABLE} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION (_nt_setup_nuget_tools P_VERSION)
    
	# Configure unpack dir
	SET(_NUGET_UNPACK_DIR "$ENV{NUGET_UNPACK_DIR}")

	IF (NOT _NUGET_UNPACK_DIR)
		SET(_NUGET_UNPACK_DIR $ENV{TMP})
	ENDIF()

	FILE(MAKE_DIRECTORY ${_NUGET_UNPACK_DIR})
	
	FILE(TO_CMAKE_PATH ${_NUGET_UNPACK_DIR} _NUGET_UNPACK_DIR)
	
	SET(NUGET_UNPACK_DIR ${_NUGET_UNPACK_DIR} CACHE INTERNAL "Folder where nuget packages will be unpacked.")
	
	_nt_message (STATUS "Using NUGET_UNPACK_DIR = ${_NUGET_UNPACK_DIR}")
	
	# Find nuget executable
	UNSET(_NUGET_EXECUTABLE CACHE)
	
	_nt_search_coapp_nuget_executable (_NUGET_EXECUTABLE ${_NUGET_UNPACK_DIR} "${P_VERSION}")
	
	IF (EXISTS "${_NUGET_EXECUTABLE}")
		_nt_message (STATUS "Nuget found in ${_NUGET_EXECUTABLE}. Let's roll!")
	ELSE()
		_nt_message (STATUS "Nuget not found. Installing it...")
		
		_nt_install_nuget (_NUGET_EXECUTABLE "${_NUGET_UNPACK_DIR}" "${P_VERSION}")
	ENDIF()

    IF (_NUGET_EXECUTABLE)
	    SET (NUGET_EXECUTABLE ${_NUGET_EXECUTABLE} CACHE INTERNAL "Full path to nuget executable.")
    ELSE()
        IF (P_VERSION)
            _nt_message (FATAL_ERROR "Could not find or install nuget with CMake integration version ${P_VERSION}.")
        ELSE()
            _nt_message (FATAL_ERROR "Could not find or install nuget with CMake integration.")
        ENDIF()
    ENDIF()
	
    GET_FILENAME_COMPONENT (_DIR ${NUGET_EXECUTABLE} DIRECTORY)
    INCLUDE (${_DIR}/NugetCore.cmake)
    
	# Reset global variables
	SET(_NUGET_DEPS "" CACHE INTERNAL "List of declared nuget dependencies" FORCE)
	SET(_NUGET_LOCAL_DEPS "" CACHE INTERNAL "List of nugets declared in this project (disregarding nuget and project transitivity)." FORCE)
	SET(_NUGET_DECLARING_OTHER_PROJECT FALSE CACHE INTERNAL "Temporary variable used to set whether the current NugetDeps file is from another project." FORCE)
			
	# Load dependencies
	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NugetDeps.cmake)
		INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/NugetDeps.cmake)
	ENDIF()
ENDFUNCTION()

_nt_setup_nuget_tools( "${Nuget_FIND_VERSION}" )