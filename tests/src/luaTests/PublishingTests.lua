function publishTest()
	local transportObj = co.new "mockReef.Transport"
	local transport = transportObj.transport
	local serverANodeObj = co.new "reef.Node"
	serverANodeObj.transport = transport
	
	local cliNodeObj = co.new "reef.Node"
	cliNodeObj.transport = transport
	
	local serverBNodeObj = co.new "reef.Node"
	serverBNodeObj.transport = transport
	
	local server = serverANodeObj.node;
	local clientA = cliNodeObj.node
	local clientB = serverBNodeObj.node
	transportObj.node = server
	transportObj.node = clientA
	transportObj.node = clientB
	
	server:start( "addressServer", "addressServer" )   
	
	clientA:start( "addressA", "addressA" )
	
	clientB:start( "addressB", "addressB" )
	
	--[[ Testes the access to the published instance ]]
	
	-- creates a local instance, publishes it and sets a field for later on testing
	local instanceTC = co.new "moduleA.TestComponent"
	EXPECT_EQ( server:publishInstance( instanceTC, "key" ), 1 )
	local simpleTypes = instanceTC.simple
	simpleTypes.storedInt = 5
	
	-- Gets the published instance proxy in the clients
	local instanceTCinA = clientA:findRemoteInstance( "moduleA.TestComponent", "key", "addressServer" )
	local instanceTCinB = clientB:findRemoteInstance( "moduleA.TestComponent", "key", "addressServer" )
	
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
	EXPECT_EQ( server:getInstance( 1 ), instanceTC )
	
	-- Checks if clientA and clientB are referers
	local references = server:getRemoteReferences( 1 )
	EXPECT_EQ( references, 3 )
	
	-- Checks if returns 0 in case of an invalid id
	references = server:getRemoteReferences( 3 )
	EXPECT_EQ( references, 0 )
	
	-- Checks if a remotely created instance is accessible locally
	local newInstProxy = clientA:newRemoteInstance( "moduleA.TestComponent", "addressServer" )
	newInstProxy.simple.storedInt = 8
	local newInst = server:getInstance( 2 )
	EXPECT_EQ( newInst.simple.storedInt, 8 )
	
	-- Checks if clientA is referer
	references = server:getRemoteReferences( 2 )
	EXPECT_EQ( references, 1 )
	
	-- Checks if the ID counting is correct
	clientB:newRemoteInstance( "moduleA.TestComponent", "addressServer" )
	clientB:newRemoteInstance( "moduleA.TestComponent", "addressServer" )
	clientB:newRemoteInstance( "moduleA.TestComponent", "addressServer" )
	newInstProxy = clientB:newRemoteInstance( "moduleA.TestComponent", "addressServer" )
	newInstProxy.simple.storedInt = 9
	newInst = server:getInstance( 6 )
	EXPECT_EQ( newInst.simple.storedInt, 9 )
	
	-- Checks if clientA is referer
	references = server:getRemoteReferences( 6 )
	EXPECT_EQ( references, 1 )
end

