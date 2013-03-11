local M = {}
local cache = {}
local newObjectIds = {}
require "flow.GraphIds"

function M.initializeIds( space )
	cache[space] = GraphIds:new( space )
end

function getCache( space )
	return cache[space]
end

function M.applyReceivedChanges( graph, newObjects, changeSets )
	newObjectIds = {}
	applyReceivedNewObjects( graph, newObjects )
	applyReceivedChangeSet( graph, changeSets )
end

function checkNewInGraph( service )
	return newObjectIds[ service.provider ]
end

function applyReceivedChangeSet( graph, changeSets )
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
	
	for i, changeSet in ipairs( changeSets ) do
		service = getCache( graph ):getService( changeSet.serviceId )
		if not checkNewInGraph( service ) then
			graph:addChange( service )
		end
	end
	graph:notifyChanges()
end

function applyReceivedNewObjects( graph, newObjects )
	for i, newObject in ipairs( newObjects ) do
		local newObjectCoral = co.new( newObject.typeName )
		getCache( graph ):objectId( newObjectCoral, true )
		newObjectIds[ newObjectCoral ] = true
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
			local serviceRef
			for i, id in ipairs( idList ) do
				serviceRef = getCache( graph ):getService( id )
				refVec[ #refVec+1 ] = serviceRef
			end
			service[memberName] = refVec
		else
			local chunk = load( "return " .. str:sub( 2 ) )
			local id = chunk()
			local serviceRef =  getCache( graph ):getService( id )
			
			service[memberName] = serviceRef
		end
	end
end

return M