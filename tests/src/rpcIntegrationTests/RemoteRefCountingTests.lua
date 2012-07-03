function bulkReferencing()

	local setup = co.new "rpcTests.TestSetup".setup
	setup:initTest( 10 )
	
	setup:publishForAll( "rpcTests.TestComponent", "key" )
	
	-- Cascade calling test
	local ref = {}
	local client = setup:getNode( 1 )
	for i = 2, 10 do
		ref[i] = client:findRemoteInstance( "rpcTests.TestComponent", "key"..i, "address"..i ).reference
	end
	
	local clientRef = client:getInstance( 0 ).reference
	local simple = setup:getNode( 6 ):getInstance( 0 ).simple
	simple.storedInt = 6
	
	EXPECT_EQ( clientRef:meth1( ref[2], ref[3], ref[4], ref[5], simple ), 7 )

	-- Ping Pong test
	local tcIn2 = client:newRemoteInstance( "rpcTests.TestComponent", "address2" )
	tcIn2.simple.storedInt = 1

	EXPECT_EQ( clientRef:meth1( ref[2], clientRef, tcIn2.reference, clientRef, tcIn2.simple ), 2 )

	simple = ref[2].provider.simple
	simple.storedInt = 2
	EXPECT_EQ( clientRef:meth1( ref[2], ref[3], ref[2], ref[4], simple ), 3 )

	EXPECT_EQ( clientRef:meth1( ref[2], ref[2], ref[2], ref[2], simple ), 3 )
	
	setup:tearDown()
end