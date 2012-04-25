local ActiveLink = co.Component( { name = "mockReef.ActiveLink", 
								provides = { active = "reef.IActiveLink" } } )

function ActiveLink:send( msg )
	--print( "ActiveLink of address ".. self.address .. " sent: " .. tostring( msg ) )
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