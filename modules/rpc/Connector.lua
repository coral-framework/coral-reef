local Connector = co.Component( { name = "rpc.Connector", 
								provides = { active = "rpc.IConnector" } } )

function Connector:send( msg )
	self.net:send( self.address, msg )
	self.net:updateAll()
end

function Connector:receiveReply()
	local msg = self.net:checkReply( self.address )
	
	if msg then
		return true, msg
	end
	
	self.net:updateAll()
	return false, ""
end

function Connector:getAddress()
	return self.address
end

return Connector