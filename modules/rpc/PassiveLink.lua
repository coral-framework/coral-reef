local PassiveLink = co.Component( { name = "rpc.PassiveLink", 
								provides = { passive = "rpc.IPassiveLink" } } )

function PassiveLink:sendReply( msg )
	self.net:sendReply( self.address, msg )
end

function PassiveLink:receive()
	local msg = self.net:check( self.address )
	
	if msg then
		return true, msg
	end
	
	return false, ""
end

function PassiveLink:getAddress()
	return self.address
end

return PassiveLink