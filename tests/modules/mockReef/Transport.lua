local ActiveLink = require "mockReef.ActiveLink"
local PassiveLink = require "mockReef.PassiveLink"

local Transport = co.Component( "mockReef.Transport" )

local net__index = {}

function net__index:updateAll()
	for i = 1, #self.nodes do
		self.nodes[i]:update()
	end
end

function net__index:send( address, msg )
	self.msgs[address] = msg 
end

function net__index:checkReply( address )
	local reply = self.replies[address]
	self.replies[address] = nil
	
	return reply
end

function net__index:sendReply( address, msg )
	self.replies[address] = msg
end

function net__index:check( address )
	local msg = self.msgs[address]
	self.msgs[address] = nil
	
	return msg
end

local net = setmetatable( { msgs = {}, replies = {}, nodes = {} }, { __index = net__index } )

function Transport:__init()
	self.openLinks = {}
end

function Transport:bind( address )
	return PassiveLink { net = net, address = address }.passive
end

function Transport:connect( address )
	local link = self.openLinks[address]
	
	if not link then
		link = ActiveLink { net = net, address = address }.active
		self.openLinks[address] = link
	end
	return link
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
	net = setmetatable( { msgs = {}, replies = {}, nodes = {} }, { __index = net__index } )
end

function Transport:removeNodesFromNetwork()
	for i,v in ipairs( self.addedNodes ) do
		table.remove( net.nodes, index )
	end
end


return Transport