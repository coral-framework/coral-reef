local PassiveLink = co.Component( { name = "testTransport.PassiveLink", 
								provides = { passive = "reef.IPassiveLink" } } )

function PassiveLink:sendReply( msg )
	self.net:sendReply( self.address, msg )
end

function PassiveLink:receive()
	local msg = self.net:check( self.address )
	--print( "PassiveLink of address ".. self.address .. " received: " .. tostring( msg ) )
	
	if msg then
		return true, msg
	end
	
	return false, ""
end

function PassiveLink:getAddress()
	return self.address
end

return PassiveLink