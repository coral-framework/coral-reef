local M = {}
local cache = {}
require "dso.GraphIds"

function M.initializeIds( space )
	cache[space] = GraphIds:new( space )
end

function getCache( space )
	return cache[space]
end

function M.applyReceivedChangeSet( graph, changeSets )
	local service
	for i, changeSet in ipairs( changeSets ) do
		for j, change in ipairs( changeSet ) do
			service = getCache( graph ):getService( changeSet.serviceId )
			if change.newRefValue == nil then
				service[ change.name ] = change.newValue
			else
				local str = change.newRefValue
				applyRefChange( service, str )
			end
		end
	end
end

function M.applyReceivedChange( graph, serviceId, memberName, newValue )
	local service = getCache( graph ):getService( serviceId )
	service[memberName] = newValue
	
end

function M.applyReceivedNewObject( graph, newId, newType )
	local newObject = co.new( newType )
	getCache( graph ):objectId( newObject )
end

function M.applyReceivedRefChange( graph, serviceId, memberName, newRefValue )
	local service
	service = getCache( graph ):getService( serviceId )
	applyRefChange( graph, service, memberName, newRefValue )
end

function applyRefChange( graph, service, memberName, str )
	if str:sub(1,1) == '#' then
		if str:sub(2,2) == '{' then
			--refVec
			local refVec = {}
			local chunk = load( "return " .. str:sub( 2 ) )
			local idList = chunk()
			
			for i, id in ipairs( idList ) do
				refVec[ #refVec+1 ] = getCache( graph ):getService( id )
			end
			service[memberName] = refVec
		else
			local chunk = load( "return " .. str:sub( 2 ) )
			local id = chunk()
			
			service[memberName] = getCache( graph ):getService( id )
		end
	end
end

return M