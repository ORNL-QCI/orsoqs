# Copyright 2015 Ronald Sadlier - Oak Ridge National Laboratory
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#



# This can be made more robust in the future, but for now this is good enough
set(installed_files
	/usr/local/lib/liborsoqs.so
	/usr/lib/python2.7/dist-packages/_orsoqs.so
	/usr/lib/python2.7/dist-packages/orsoqs.py
	/usr/lib/python2.7/site-packages/_orsoqs.so
	/usr/lib/python2.7/site-packages/orsoqs.py)

foreach(cur_file ${installed_files})
	if(EXISTS ${cur_file})
		message(STATUS "Removing file: '${cur_file}'")
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${cur_file}
				OUTPUT_VARIABLE rm_out
				RESULT_VARIABLE rm_retval)
		if(NOT ${rm_retval} STREQUAL 0)
			message(FATAL_ERROR "Failed to remove file: '${cur_file}'. Do you have permission?")
		endif()
	else()
		message(STATUS "File '${cur_file}' does not exist.")
	endif()
endforeach(cur_file installed_files)