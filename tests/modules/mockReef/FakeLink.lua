local FakeLink = co.Component( "mockReef.FakeLink" )

-- IActiveLink methods
function FakeLink:send( msg )
	self.msg = msg
end

function FakeLink:receiveReply()
	return true, self.reply
end

function FakeLink:getAddress()
	return self.address
end

-- IFakeLink methods

function FakeLink:setAddress( address )
	self.address = address
end

function FakeLink:setReply( reply )
	self.reply = reply
end

function FakeLink:getMsg()
	return self.msg
end

return FakeLink