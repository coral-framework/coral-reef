local M = {}

local cache = {}


require "flow.GraphIds"

require "table"

function M.initializeIds( space )
	cache[space] = GraphIds:new( space )
end

local function getCache( space )
	return cache[space]
end

local function getModel( space )
	return space.universe.model
end


function M.processAllSpaceChanges( space, allSpaceChanges, observers )
	if #observers == 0 then
		return
	end

	local accumulatedChanges = {}
	local newObjects = {}

	for i, spaceChanges in ipairs( allSpaceChanges ) do
		processNewObjects( space, spaceChanges, newObjects )
	end
	
	local newObjectStructs = generateValueForNewObjects( space, newObjects, accumulatedChanges )
	
	for i, spaceChanges in ipairs( allSpaceChanges ) do
		processChanges( space, spaceChanges, accumulatedChanges )
	end

	local changeSetArray = {}
	for i, changes in pairs( accumulatedChanges ) do
		local changeSet = co.new "flow.ChangeSet"
		changeSet.serviceId = i
		changeSet.changes = changes
		
		changeSetArray[ #changeSetArray + 1 ] = changeSet
	end
	
	for _, observer in ipairs( observers ) do
		if #newObjectStructs > 0 then
			table.sort( newObjectStructs, function( x, y ) return x.newId < y.newId end )
			--observer:onNewObjects( newObjectStructs )
		end
		
		if #changeSetArray > 0 then
			table.sort( changeSetArray, function( x, y ) return x.serviceId < y.serviceId end )
			--observer:onRemoteSpaceChanged( changeSetArray )
		end
		observer:onRemoteSpaceChanged( newObjectStructs, changeSetArray )
	end
end

function processNewObjects( space, spaceChanges, newObjectsList )
	local newObjects = spaceChanges.addedObjects
	for i, newObject in ipairs( newObjects ) do
		newObjectsList[ newObject ] = true
	end
	local removedObjects = spaceChanges.removedObjects
	for i, removed in ipairs( removedObjects ) do
		newObjectsList[ removed ] = nil
		getCache( space ):removeObject( removed )
	end
end

function generateValueForNewObjects( space, newObjects, resultChangesTable )
	local newObjectResult = {}
	
	for newObject, _ in pairs( newObjects ) do
		getCache( space ):objectId( newObject )
		
		local newObjectStruct = co.new "flow.NewObject"
		local objectId = getCache( space ):getId( newObject )
		newObjectStruct.newId = objectId
		newObjectStruct.typeName = newObject.component.fullName
		newObjectResult[ #newObjectResult + 1 ] = newObjectStruct
		
		getNewObjectChanges( space, newObject, resultChangesTable )
	end
	
	return newObjectResult
end

function processChanges( space, spaceChanges, resultChangesTable )
	for i, objectChange in ipairs( spaceChanges.changedObjects ) do
		processObjectChanges( space, objectChange, resultChangesTable )
	end
end

function getNewObjectChanges( space, newObject, resultChangesTable )
	
	local objectId = getCache( space ):getId( newObject )
	local ports = getModel( space ):getPorts( newObject.component )
	local receptacleChanges = resultChangesTable[ objectId ] or {}
	for i, port in ipairs( ports ) do
		if port.isFacet then
			getNewServiceChanges( space, newObject[port.name], resultChangesTable )
		else
			local value = newObject[ port.name ]
			if value ~= nil then
				receptacleChanges[ #receptacleChanges + 1 ] = createChange( space, port, value )
			end
		end
	end
	if #receptacleChanges > 0 then
		resultChangesTable[ objectId ] = receptacleChanges
	end
end

function getNewServiceChanges( space, newService, resultChangesTable )
	local serviceId = getCache( space ):getId( newService )
	local fields = getModel( space ):getFields( newService.interface )
	
	local fieldChanges = resultChangesTable[ serviceId ] or {}
	local fieldValue
	for i, field in ipairs( fields ) do
		fieldValue = newService[ field.name ]
		if fieldValue ~= nil then
			fieldChanges[ #fieldChanges + 1] = createChange( space, field, fieldValue )
		end
	end
	if #fieldChanges > 0 then
		resultChangesTable[ serviceId ] = fieldChanges
	end
end

function processObjectChanges( space, objectChanges, resultChangesTable )
	if not getCache( space ):hasId( objectChanges.object ) then
		return
	end
	local objectId = getCache( space ):getId( objectChanges.object )
	local connectionChanges = {}
	
	for i, connChange in ipairs( objectChanges.changedConnections ) do
		connectionChanges[ #connectionChanges+1 ] = createChange( space, connChange.receptacle, connChange.current )
	end
	if #connectionChanges > 0 then
		resultChangesTable[ objectId ] = connectionChanges
	end
	
	for i, changedService in ipairs( objectChanges.changedServices ) do
		processServiceChanges( space, changedService, resultChangesTable )
	end
end

function processServiceChanges( space, serviceChanges, resultChangesTable )
	local serviceId = getCache( space ):getId( serviceChanges.service )
	local allChanges = resultChangesTable[ serviceId ] or {}
	
	local changedValueFields = serviceChanges.changedValueFields 
	for i, value in ipairs( changedValueFields ) do
		allChanges[#allChanges+1] = createChange( space, value.field, value.current )
	end
	
	for i, ref in ipairs( serviceChanges.changedRefFields ) do
		allChanges[#allChanges+1] = createChange( space, ref.field, ref.current )
	end
	
	for i, refVecChange in ipairs( serviceChanges.changedRefVecFields ) do
		allChanges[#allChanges+1] = createChange( space, refVecChange.field, refVecChange.current )
	end
	
	if #allChanges > 0 then
		resultChangesTable[ serviceId ] = allChanges
	end
end

function createChange( space, member, value )
	
	change = co.new "flow.Change"
	change.name = member.name
	
	if member.type.kind == 'TK_INTERFACE' then
		if value == nil then
			change.newRefValue = "nil"
		else
			change.newRefValue = "#" .. getCache( space ):getId( value )
		end
	elseif member.type.kind == 'TK_ARRAY' and member.type.elementType.kind == 'TK_INTERFACE' then
		local refVecStr = "#{"
		for i, refVecService in ipairs( value ) do
			refVecStr = refVecStr .. getCache( space ):getId( refVecService ) .. ","
		end
		refVecStr = refVecStr .. "}"
		change.newRefValue = refVecStr
	else
		change.newValue = value
	end
	return change
end

return M
