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
		service = getCache( graph ):getService( changeSet.serviceId )
		for i, change in ipairs( changeSet.changes ) do
			if change.newRefValue == nil or change.newRefValue == "" then
				service[ change.name ] = change.newValue
			else
				local str = change.newRefValue
				applyRefChange( graph, service, change.name, str )
			end
		end
	end
end

function M.applyReceivedNewObject( graph, newId, newType )
	local newObject = co.new( newType )
	getCache( graph ):objectId( newObject )
end

function M.applyReceivedNewObjects( graph, newObjects )
	for i, newObject in ipairs( newObjects ) do
		local newObjectCoral = co.new( newObject.typeName )
		getCache( graph ):objectId( newObjectCoral )
		if newObject.newId ~= getCache( graph ):getId( newObjectCoral ) then
			error( "graph inconsistent" )
		end
	end
end

function applyRefChange( graph, service, memberName, str )
	if str == "nil" then
		service[memberName] = nil
		
	elseif str:sub(1,1) == '#' then
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