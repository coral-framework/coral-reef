local expected = 
	{
		["BACABA"] = "10.0.24.75:5551",
		["BACABA:50"] = "10.0.24.75:50",
		["10.0.24.75:50"] = "10.0.24.75:50",
		[""] = "10.0.24.75:5551",
		["*:5000"] = "10.0.24.75:5000",
		["TARUMA"] = "10.0.24.122:5551",
		["localhost"] = "127.0.0.1:5551",
		["localhost:32"] = "127.0.0.1:32"
	}
	
local errors = 
{
	"SBRUBBLES:5000",
	"10.3.4:221",
	"10.0.24.256"
}

local dns = require "rpc.AddressResolver"
	
function test()
	ASSERT_FALSE( dns == nil )
	
	for input, expected in pairs( expected ) do
		local actual = dns.resolveAddress( input )
		EXPECT_EQ( expected, actual )
	end
	
	for _,input in ipairs( errors ) do
		local closure = 
		function()
			dns.resolveAddress( input )
		end
		EXPECT_EXCEPTION( "co.IllegalArgumentException", "invalid", closure )
	end
	
end
