function bulkReferencing()

	local transportObj = co.new "mockReef.Transport"
	local transport = transportObj.transport
	
	local hosts = {}
	local numHosts = 10
	-- Create 10 nodes and setup their use of the mockReef transport
	for i = 1, numHosts do
		local nodeObj = co.new "reef.rpc.Node"
		nodeObj.transport = transport
		hosts[i] = {}
		local host = hosts[i]
		
		host.node = nodeObj.node
		transportObj.node = host.node
		host.node:start( "address" .. tostring( i ), "address" .. tostring( i ) )
	end
	
	for i = 1, numHosts do
		local host = hosts[i]

		host.tcLocal = co.new "moduleA.TestComponent"
		host.tcLocal.simple.storedInt = i
		host.node:publishInstance( host.tcLocal, "key" .. tostring( i ) )
	end
	
	-- Cascade calling test
	hosts[1].remoteTCs = {}
	for i = 2, numHosts do
		hosts[1].remoteTCs[i] = hosts[1].node:findRemoteInstance( "moduleA.TestComponent", "key" .. tostring( i ), 
										"address" .. tostring( i ) )
	end
	local ref1 = hosts[1].remoteTCs[6].reference
	local ref2 = hosts[1].remoteTCs[2].reference
	local ref3 = hosts[1].remoteTCs[3].reference
	local ref4 = hosts[1].remoteTCs[4].reference
	local ref5 = hosts[1].remoteTCs[5].reference
	local simple = hosts[6].tcLocal.simple
	
	EXPECT_EQ( ref1:meth1( ref2, ref3, ref4, ref5, simple ), 7 )
	
	-- Ping Pong test
	local ref22 = hosts[1].node:newRemoteInstance( "moduleA.TestComponent",	"address2" ).reference
	
	simple = hosts[1].tcLocal.simple
	EXPECT_EQ( ref1:meth1( ref2, ref1, ref22, ref1, simple ), 2 )
	
	simple = hosts[1].remoteTCs[2].simple
	EXPECT_EQ( ref1:meth1( ref2, ref3, ref2, ref4, simple ), 3 )
	
	EXPECT_EQ( ref1:meth1( ref2, ref2, ref2, ref2, simple ), 3 )
end
