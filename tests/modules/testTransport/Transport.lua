local ActiveLink = require "testTransport.ActiveLink"
local PassiveLink = require "testTransport.PassiveLink"

local Transport = co.Component( "testTransport.Transport" )

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

function Transport:__init()
	self.net = setmetatable( { msgs = {}, replies = {}, nodes = {} }, { __index = net__index } )
end

function Transport:bind( address )
	return PassiveLink { net = self.net, address = address }.passive
end

function Transport:connect( address )
	return ActiveLink { net = self.net, address = address }.active
end

function Transport:setNodeService( node )
	self.net.nodes[#self.net.nodes+1] = node
end

function Transport:getNodeService()
	return nil
end
return Transport