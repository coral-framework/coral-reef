local ActiveLink = co.Component( { name = "reef.rpc.ActiveLink", 
								provides = { active = "reef.rpc.IActiveLink" } } )

function ActiveLink:send( msg )
	self.net:send( self.address, msg )
	self.net:updateAll()
end

function ActiveLink:receiveReply()
	local msg = self.net:checkReply( self.address )
	
	if msg then
		return true, msg
	end
	
	self.net:updateAll()
	return false, ""
end

function ActiveLink:getAddress()
	return self.address
end

return ActiveLink