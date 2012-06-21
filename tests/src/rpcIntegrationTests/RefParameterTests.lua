
function test()
	local setup = co.new "mockReef.TestSetup".setup
	setup:initTest( 3 )
	
	local client = setup:getNode( 3 )
	print( "aaaa" )
	local instanceIn1 = client:newRemoteInstance( "moduleA.TestComponent",	"address1" )
	local refTypesServiceIn1 = instanceIn1.reference
	local simpleTypesServiceIn1 = instanceIn1.simple
	
	local instanceIn2 = client:newRemoteInstance( "moduleA.TestComponent", "address2" )
	local simpleTypesServiceIn2 = instanceIn2.simple
	local refTypesServiceIn2 = instanceIn2.reference
	
	local instanceLocal = co.new "moduleA.TestComponent"
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
	
	refTypesServiceIn1.simple = simpleTypesServiceIn2
	refTypesServiceIn2.simple = simpleTypesServiceIn1
	
	EXPECT_EQ( refTypesServiceIn1.simple, simpleTypesServiceIn2 )
	
	local localInstanceIn1 = setup:getNode( 1 ):getInstance( 1 )
	localInstanceIn1.simple.storedInt = 10
	
	EXPECT_EQ( refTypesServiceIn2.simple.storedInt, 10 )
	
	setup:tearDown()
end
