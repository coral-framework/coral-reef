local M = {}

require "dso.GraphIds"
local cache
local model

require "table"

function M.initializeIds( space )
	cache = GraphIds:new( space )
	model = space.universe.model
end

function M.processAllSpaceChanges( space, allSpaceChanges, observers )
	if #observers == 0 then
		return
	end

	local accumulatedChanges = {}
	local newObjects = {}

	for i, spaceChanges in ipairs( allSpaceChanges ) do
		processNewObjects( spaceChanges, newObjects )
	end
	
	local newObjectStructs = generateValueForNewObjects( newObjects, accumulatedChanges )
		
	for i, spaceChanges in ipairs( allSpaceChanges ) do
		processChanges( spaceChanges, accumulatedChanges )
	end
	
	local changeSetArray = {}
	for i, changes in pairs( accumulatedChanges ) do
			
		local changeSet = co.new "dso.ChangeSet"
		changeSet.serviceId = i
		changeSet.changes = changes
		
		changeSetArray[ #changeSetArray + 1 ] = changeSet
	end
	
	for _, observer in ipairs( observers ) do
		if #newObjectStructs > 0 then
			table.sort( newObjectStructs, function( x, y ) return x.newId < y.newId end )
			observer:onNewObjects( newObjectStructs )
		end
		
		if #changeSetArray > 0 then
			observer:onRemoteSpaceChanged( changeSetArray )
		end
	end
end

function processNewObjects( spaceChanges, newObjectsList )
	local newObjects = spaceChanges.addedObjects
	for i, newObject in ipairs( newObjects ) do
		newObjectsList[ newObject ] = true
	end
	local removedObjects = spaceChanges.removedObjects
	for i, removed in ipairs( removedObjects ) do
		newObjectsList[ removed ] = nil
	end
end

function generateValueForNewObjects( newObjects, resultChangesTable )
	
	local newObjectResult = {}
	
	for newObject, _ in pairs( newObjects ) do
		cache:objectId( newObject )
		
		local newObjectStruct = co.new "dso.NewObject"
		local objectId = cache:getId( newObject )
		newObjectStruct.newId = objectId
		newObjectStruct.typeName = newObject.component.fullName
		newObjectResult[ #newObjectResult + 1 ] = newObjectStruct
		
		getNewObjectChanges( newObject, resultChangesTable )
	end
	
	return newObjectResult
end

function processChanges( spaceChanges, resultChangesTable )
	for i, objectChange in ipairs( spaceChanges.changedObjects ) do
		processObjectChanges( objectChange, resultChangesTable )
	end
end

function getNewObjectChanges( newObject, resultChangesTable )
	
	local objectId = cache:getId( newObject )
	local ports = model:getPorts( newObject.component )
	local receptacleChanges = resultChangesTable[ objectId ] or {}
	for i, port in ipairs( ports ) do
		if port.isFacet then
			getNewServiceChanges( newObject[port.name], resultChangesTable )
		else
			local value = newObject[ port.name ]
			if value ~= nil then
				receptacleChanges[ #receptacleChanges + 1 ] = createChange( port, value )
			end
		end
	end
	if #receptacleChanges > 0 then
		resultChangesTable[ objectId ] = receptacleChanges
	end
end

function getNewServiceChanges( newService, resultChangesTable )
	local serviceId = cache:getId( newService )
	local fields = model:getFields( newService.interface )
	
	local fieldChanges = resultChangesTable[ serviceId ] or {}
	local fieldValue
	for i, field in ipairs( fields ) do
		fieldValue = newService[ field.name ]
		if fieldValue ~= nil then
			fieldChanges[ #fieldChanges + 1] = createChange( field, fieldValue )
		end
	end
	if #fieldChanges > 0 then
		resultChangesTable[ serviceId ] = fieldChanges
	end
end

function processObjectChanges( objectChanges, resultChangesTable )
	local objectId = cache:getId( objectChanges.object )
	local connectionChanges = {}
	
	for i, connChange in ipairs( objectChanges.changedConnections ) do
		connectionChanges[ #connectionChanges+1 ] = createChange( connChange.receptacle, connChange.current )
	end
	if #connectionChanges > 0 then
		resultChangesTable[ objectId ] = connectionChanges
	end
	
	for i, changedService in ipairs( objectChanges.changedServices ) do
		processServiceChanges( changedService, resultChangesTable )
	end
end

function processServiceChanges( serviceChanges, resultChangesTable )
	local serviceId = cache:getId( serviceChanges.service )
	local allChanges = resultChangesTable[ serviceId ] or {}
	
	local changedValueFields = serviceChanges.changedValueFields 
	for i, value in ipairs( changedValueFields ) do
		allChanges[#allChanges+1] = createChange( value.field, value.current )
	end
	
	for i, ref in ipairs( serviceChanges.changedRefFields ) do
		allChanges[#allChanges+1] = createChange( ref.field, ref.current )
	end
	
	for i, refVecChange in ipairs( serviceChanges.changedRefVecFields ) do
		allChanges[#allChanges+1] = createChange( refVecChange.field, refVecChange.current )
	end
	
	if #allChanges > 0 then
		resultChangesTable[ serviceId ] = allChanges
	end
end

function createChange( member, value )
	
	change = co.new "dso.Change"
	change.name = member.name
	
	if member.type.kind == 'TK_INTERFACE' then
		if value == nil then
			change.newRefValue = "nil"
		else
			change.newRefValue = "#" .. cache:getId( value )
		end
	elseif member.type.kind == 'TK_ARRAY' and member.type.elementType.kind == 'TK_INTERFACE' then
		local refVecStr = "#{"
		for i, refVecService in ipairs( value ) do
			refVecStr = refVecStr .. cache:getId( refVecService ) .. ","
		end
		refVecStr = refVecStr .. "}"
		change.newRefValue = refVecStr
	else
		change.newValue = value
	end
	return change
end

return M
