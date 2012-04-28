-- Tests for massive remote instantiation and ref type passing via parameters
--[[
local transportObj = co.new "mockReef.Transport"
local transport = transportObj.transport

local hosts = {}
local numHosts = 10
-- Create 10 nodes and setup their use of the mockReef transport
for i = 1, numHosts do
	local nodeObj = co.new "reef.Node"
	nodeObj.transport = transport
	hosts[i] = {}
	
	
	host.node = nodeObj.node
	transportObj.node = host.node
	host.node:start( "address" .. tostring( i ), "address" .. tostring( i ) )
end

for i = 1, numHosts do
	local host = hosts[i]
	
	host.moduleA = host.node:newInstance(
end

]]