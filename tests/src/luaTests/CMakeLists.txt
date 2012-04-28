################################################################################
# Run the test scripts with CTest
################################################################################

CORAL_ADD_TEST( luaTests testkit.Run ${CMAKE_CURRENT_SOURCE_DIR} --shuffle -o "output/LuaTests$<CONFIGURATION>.xml" )
