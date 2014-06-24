
function test()
	local setup = co.new "stubs.TestSetup".setup
	setup:initTest( 3 )
	
	local serverA = setup:getNode( 1 )
	local serverB = setup:getNode( 2 )
	local client = setup:getNode( 3 )
	
	local instanceIn1Orig = co.new( "stubs.TestComponent" )
	serverA:publishInstance( instanceIn1Orig, "instanceIn1" )
	
	local instanceIn1 = client:findRemoteInstance( "stubs.TestComponent", "instanceIn1", "address1" )
	local refTypesServiceIn1 = instanceIn1.reference
	local simpleTypesServiceIn1 = instanceIn1.simple
	
	local instanceIn2Orig = co.new( "stubs.TestComponent" )
	serverB:publishInstance( instanceIn1Orig, "instanceIn2" )
	
	local instanceIn2 = client:findRemoteInstance( "stubs.TestComponent", "instanceIn2", "address2" )
	local simpleTypesServiceIn2 = instanceIn2.simple
	local refTypesServiceIn2 = instanceIn2.reference
	
	local instanceLocal = co.new "stubs.TestComponent"
	local simpleTypesServiceLocal = instanceLocal.simple
	
	EXPECT_EQ( refTypesServiceIn1:callIncrementInt( simpleTypesServiceIn1, 3 ), 4 )
	EXPECT_EQ( refTypesServiceIn1:callDivideDouble( simpleTypesServiceIn1, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceIn1:concatenateString( simpleTypesServiceIn1, "aaa", "bbb" ), "aaabbb" )
	
	EXPECT_EQ( refTypesServiceIn1:callIncrementInt( simpleTypesServiceLocal, 3 ), 4 )
	EXPECT_EQ( refTypesServiceIn1:callDivideDouble( simpleTypesServiceLocal, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceIn1:concatenateString( simpleTypesServiceLocal, "aaa", "bbb" ), "aaabbb" )
	
	EXPECT_EQ( refTypesServiceIn1:callIncrementInt( simpleTypesServiceIn2, 3 ), 4 )
	EXPECT_EQ( refTypesServiceIn1:callDivideDouble( simpleTypesServiceIn2, 15, 5 ), 3 )
	EXPECT_EQ( refTypesServiceIn1:concatenateString( simpleTypesServiceIn2, "aaa", "bbb" ), "aaabbb" )
	
	refTypesServiceIn1:setSimple( simpleTypesServiceIn2 )
	refTypesServiceIn2:setSimple( simpleTypesServiceIn1 )
	
	local localInstanceIn1 = setup:getNode( 1 ):getInstance( 0 )
	localInstanceIn1.simple.storedInt = 10
	
	EXPECT_EQ( refTypesServiceIn2:getSimple().storedInt, 10 )
	
	local refTypesIn1Local = refTypesServiceIn1:getSimple()
	
	EXPECT_EQ( refTypesIn1Local.storedInt, simpleTypesServiceIn2.storedInt )
	EXPECT_EQ( refTypesIn1Local.storedString, simpleTypesServiceIn2.storedString )
	
	setup:tearDown()
end
