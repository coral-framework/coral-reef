local M = {}

local dns = (require "socket").dns

function M.resolveAddress( address )
	
    if address == nil or address == "" then
        co.raise( "co.IllegalArgumentException", 'empty ip' )
    end
    
	local regexBeforePort = "([^:]*)"
	local regexPort = ":(%d+)"
	
	local nameOrIp = address:match( regexBeforePort )
	local port = address:match( regexPort )
	
    if nameOrIp == "0.0.0.0" then
        co.raise( "co.IllegalArgumentException", 'invalid ip' )
    end
    
	local resolveHostNameToIp = false
	if nameOrIp == "*" then
		nameOrIp = dns.gethostname()
		resolveHostNameToIp = true
	else
		-- try to match ip
		
		local ipRegex = "(%d+).(%d+).(%d+).(%d+)"
		local parts = table.pack( nameOrIp:match( ipRegex ) )
		
		if parts == nil or #parts ~= 4 then 
			resolveHostNameToIp = true
		else
			for _, part in ipairs( parts ) do
				if tonumber( part ) > 255 then
					co.raise( "co.IllegalArgumentException", 'invalid ip' )
				end
			end
		end
		resolveHostNameToIp = true
	end
	local resolvedIp = nameOrIp
	if resolveHostNameToIp then
		resolvedIp = dns.toip( resolvedIp )
	end
	
	if resolvedIp == nil then
		co.raise( "co.IllegalArgumentException", 'invalid hostname' )
	end
	
	if port == nil then
		port = 5551
	end
	
	return resolvedIp .. ':' .. port
	
end

return M