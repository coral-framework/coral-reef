local Acceptor = co.Component( { name = "rpc.Acceptor", 
								provides = { passive = "rpc.IAcceptor" } } )

function Acceptor:sendReply( msg )
	self.net:sendReply( self.address, msg )
end

function Acceptor:receive()
	local msg = self.net:check( self.address )
	
	if msg then
		return true, msg
	end
	
	return false, ""
end

function Acceptor:getAddress()
	return self.address
end

return Acceptor