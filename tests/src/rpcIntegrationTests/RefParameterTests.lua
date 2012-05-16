
function test()
	local setup = co.new "mockReef.TestSetup".setup
	setup:initTest( 3 )
	
	local client = setup:getNode( 3 )
	
	local instanceInA = client:newRemoteInstance( "moduleA.TestComponent",	"address1" )
	local refTypesServiceInA = instanceInA.reference
	local simpleTypesServiceInA = instanceInA.simple
	
	local instanceInB = client:newRemoteInstance( "moduleA.TestComponent", "address2" )
	local simpleTypesServiceInB = instanceInB.simple
	
	local instanceLocal = co.new "moduleA.TestComponent"
	local simpleTypesServiceLocal = instanceLocal.simple
	
	EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceInA, 3 ), 4 )
	EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceInA, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceInA, "aaa", "bbb" ), "aaabbb" )
	
	EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceLocal, 3 ), 4 )
	EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceLocal, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceLocal, "aaa", "bbb" ), "aaabbb" )
	
	EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceInB, 3 ), 4 )
	EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceInB, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceInB, "aaa", "bbb" ), "aaabbb" )
	
	setup:tearDown()
end
