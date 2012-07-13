local M = {}

require "dso.GraphIds"
local cache
local model

function M.initializeIds( space )
	cache = GraphIds:new( space )
	model = space.universe.model
end

function M.processAllSpaceChanges( space, allSpaceChanges, observers )
	local accumulatedChanges = {}
	local newObjects = {}

	for i, spaceChanges in ipairs( allSpaceChanges ) do
		processSpaceChanges( spaceChanges, accumulatedChanges, newObjects )
	end
	
	for _, observer in ipairs( observers ) do
		for i, newObject in ipairs( newObjects ) do
			observer:onNewObject( newObject.newId, newObject.newType )
		end
	
		for i, changeSet in pairs( accumulatedChanges ) do
			for j, change in ipairs( changeSet ) do
				if change.newRefValue == nil then
					observer:onChange( i, change.name, change.newValue )
				else
					observer:onRefChange( i, change.name, change.newRefValue )
				end
			end
		end
	end
end

function processSpaceChanges( spaceChanges, resultChangesTable, newObjectsTable )
	local newObjects = spaceChanges.addedObjects
	
	for i, newObject in ipairs( newObjects ) do
		cache:objectId( newObject )
		newObjectsTable[ #newObjectsTable + 1 ] = { newId = cache:getId( newObject ), newType = newObject.component.fullName }
	end
	
	-- put new object values like regular changes
	for i, newObject in ipairs( newObjects ) do
		getNewObjectChanges( newObject, resultChangesTable )
	end
	
	for i, objectChange in ipairs( spaceChanges.changedObjects ) do
		processObjectChanges( objectChange, resultChangesTable )
	end
	
	for i, removedObject in ipairs( spaceChanges.removedObjects ) do
		resultChangesTable[ cache:getId( removedObject ) ] = nil	
	end
end

function getNewObjectChanges( newObject, resultChangesTable )
	
	local objectId = cache:getId( newObject )
	local ports = model:getPorts( newObject.component )
	local receptacleChanges = {}
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
	
	local fieldChanges = {}
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
	local allChanges = {}
	
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
	
	change = { name = member.name }

	if member.type.kind == 'TK_INTERFACE' then
		if value == nil then
			change.newRefValue = nil
		else
			change.newRefValue = "#" .. cache:getId( value )
		end
	end
	
	if member.type.kind == 'TK_ARRAY' and member.type.elementType.kind == 'TK_INTERFACE' then
		local refVecStr = "#{"
		for i, refVecService in ipairs( value ) do
			refVecStr = refVecStr .. cache:getId( refVecService ) .. ","
		end
		refVecStr = refVecStr .. "}"
		change.newRefValue = refVecStr
	end
	
	change.newValue = value
	return change
end

return M