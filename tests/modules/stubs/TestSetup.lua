local TestSetup = co.Component( "stubs.TestSetup" )

function TestSetup:__init()
end

function TestSetup:initTest( numNodes )
	self.nodes = {}
	self.transports = {}
	
	for i = 1, numNodes do
		self.transports[i] = co.new "rpc.Transport"
		self.nodes[i] = co.new "rpc.Node"
		self.nodes[i].transport = self.transports[i].transport
		self.transports[i].node = self.nodes[i].node
		
		self.nodes[i].node:start( "address"..i, "address"..i )
	end
end

function TestSetup:getNode( index )
	if type( self.nodes[index] ) == "string" or self.nodes[index] == nil  then
		error( "Node of index " .. index .. " invalid" )
	end
	
	return self.nodes[index].node
end

function TestSetup:deleteNode( index )
	self.nodes[index] = "empty"
	self.transports[index]:removeNodesFromNetwork()
	self.transports[index] = "empty"
end

function TestSetup:publishForAll( componentType, keyPrefix )
	for i,v in ipairs( self.nodes ) do
		if type(v) == "userdata" then
			local instance = co.new( componentType )
			v.node:publishInstance( instance, keyPrefix .. tostring( i ) )
		end
	end
end

function TestSetup:spawnNode( address )
	local i = #self.nodes + 1

	if address == "" then
		address = "address" .. i
	end

	self.transports[i] = co.new "rpc.Transport"
	self.nodes[i] = co.new "rpc.Node"
	self.nodes[i].transport = self.transports[i].transport
	self.transports[i].node = self.nodes[i].node
	
	self.nodes[i].node:start( address, address )
	
	return self.nodes[i].node
end

function TestSetup:tearDown()
	if #self.transports > 0 then
		self.transports[1].config:clearNetwork()
	end
end

return TestSetup