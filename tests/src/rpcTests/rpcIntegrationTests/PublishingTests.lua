function publishTest()
	local setup = co.new "stubs.TestSetup".setup
	setup:initTest( 3 )
	
	local clientA = setup:getNode( 1 )
	local clientB = setup:getNode( 2 )
	local server = setup:getNode( 3 )
	
	--[[ Testes the access to the published instance ]]
	
	-- creates a local instance, publishes it and sets a field for later on testing
	local instanceTC = co.new "stubs.TestComponent"
	EXPECT_EQ( server:publishInstance( instanceTC, "key" ), 0 )
	local simpleTypes = instanceTC.simple
	simpleTypes.storedInt = 5
	
	-- Gets the published instance proxy in the clients
	EXPECT_FALSE( clientA:findRemoteInstance( "stubs.TestComponent", "wrongkey", "address3" ) )
	local instanceTCinA = clientA:findRemoteInstance( "stubs.TestComponent", "key", "address3" )
	local instanceTCinB = clientB:findRemoteInstance( "stubs.TestComponent", "key", "address3" )
	
	-- tests if the value matches in client A and changes it
	ASSERT_TRUE( instanceTCinA and instanceTCinB )
	local remoteSimpleTypes = instanceTCinA.simple
	EXPECT_EQ( remoteSimpleTypes.storedInt, 5 )
	remoteSimpleTypes.storedInt = 6
	
	-- tests if the value matches in client B and changes it
	remoteSimpleTypes = instanceTCinB.simple
	EXPECT_EQ( remoteSimpleTypes.storedInt, 6 )
	remoteSimpleTypes.storedInt = 7
	
	-- tests if the value matches locally
	EXPECT_EQ( simpleTypes.storedInt, 7 )
	
	--[[ Tests the Instance-ID mapping through node local functions ]]
	
	-- Checks if the function returns the correct instance
	EXPECT_EQ( server:getInstance( 0 ), instanceTC )
	
	-- Checks if clientA and clientB are referers
	local references = server:getInstanceNumLeases( 0 )
	EXPECT_EQ( references, 3 )
	
	-- Checks if returns 0 in case of an invalid id
	references = server:getInstanceNumLeases( 2 )
	EXPECT_EQ( references, 0 )
	
	-- Checks if a remotely created instance is accessible locally
	local newInstProxy = clientA:newRemoteInstance( "stubs.TestComponent", "address3" )
	newInstProxy.simple.storedInt = 8
	local newInst = server:getInstance( 1 )
	EXPECT_EQ( newInst.simple.storedInt, 8 )
	
	-- Checks if clientA is referer
	references = server:getInstanceNumLeases( 1 )
	EXPECT_EQ( references, 1 )
	
	-- Checks if the ID counting is correct
	clientB:newRemoteInstance( "stubs.TestComponent", "address3" )
	clientB:newRemoteInstance( "stubs.TestComponent", "address3" )
	clientB:newRemoteInstance( "stubs.TestComponent", "address3" )
	newInstProxy = clientB:newRemoteInstance( "stubs.TestComponent", "address3" )
	newInstProxy.simple.storedInt = 9
	newInst = server:getInstance( 5 )
	EXPECT_EQ( newInst.simple.storedInt, 9 )
	
	-- Checks if clientA is referer
	references = server:getInstanceNumLeases( 5 )
	EXPECT_EQ( references, 1 )
	
	setup:tearDown()
end

