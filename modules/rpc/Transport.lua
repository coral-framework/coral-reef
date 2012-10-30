local Connector = require "rpc.Connector"
local Acceptor = require "rpc.Acceptor"

local Transport = co.Component( "rpc.Transport" )

local net__index = {}

function net__index:updateAll()
	if self.frozen then 
		return nil 
	end
	for i = 1, #self.nodes do
		self.nodes[i]:update()
	end
end

function net__index:freeze()
	self.frozen = true
end

function net__index:send( address, msg )
	if self.frozen then 
		return nil 
	end
	self.msgs[address] = msg 
end

function net__index:checkReply( address )
	if self.frozen then 
		return nil 
	end
	local reply = self.replies[address]
	self.replies[address] = nil
	
	return reply
end

function net__index:sendReply( address, msg )
	if self.frozen then 
		return nil 
	end
	self.replies[address] = msg
end

function net__index:check( address )
	if self.frozen then 
		print( "self.frozen" ) 
		return nil 
	end
	local msg = self.msgs[address]
	self.msgs[address] = nil
	
	return msg
end

local net = setmetatable( { msgs = {}, replies = {}, nodes = {}, frozen = false }, { __index = net__index } )

function Transport:__init()
end

function Transport:bind( address )
	return Acceptor { net = net, address = address }.passive
end

function Transport:connect( address )
	return Connector { net = net, address = address }.active
end

function Transport:setNodeService( node )
 	if not self.addedNodes then
 		self.addedNodes = {}
 	end
 	local index = #net.nodes+1
 	table.insert( self.addedNodes, index )
 	
	net.nodes[index] = node
end

function Transport:getNodeService()
	return nil
end

function Transport:clearNetwork()
	net = setmetatable( { msgs = {}, replies = {}, nodes = {}, frozen = false }, { __index = net__index } )
end

function Transport:removeNodesFromNetwork()
	for i,v in ipairs( self.addedNodes ) do
		table.remove( net.nodes, index )
	end
end


return Transport