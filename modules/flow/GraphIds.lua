GraphIds = {}
local GraphIdsMT = { __index = GraphIds }

function GraphIds:new( space )
	self = {}
	self.model = space.universe.model
	
	self.objectIdMap = {}
	self.idObjectMap = {}
	self.currentId = 1
	setmetatable( self, GraphIdsMT )
	self:objectId( space.rootObject )
	return self;
end

function GraphIds:insertInMap( service, givenId )
	if givenId == nil then
		--generated
		givenId = self.currentId
		self.currentId = self.currentId + 1
	end
	self.objectIdMap[service] = givenId
	self.idObjectMap[givenId] = service
end

function GraphIds:hasId( service )
	return ( self.objectIdMap[service] ~= nil )
end

function GraphIds:getId( service )
	local id = self.objectIdMap[service]
	return id
end

function GraphIds:getService( id )
	return self.idObjectMap[id]
end

function GraphIds:removeObject( object )
	if self:hasId( object ) then
		local ports = self.model:getPorts( object.component )
		
		for i, port in ipairs( ports ) do
			local service = object[port.name]
			if service ~= nil then
				self.objectIdMap[ service ] = nil
			end
		end
		self.objectIdMap[object] = nil
	end
end

local function generateId( self, item, shallow )
	if not self:hasId( item ) then
		self:insertInMap( item )
		return false
	else
		return true
	end
	return shallow
end

local function giveId( self, item, givenIds )
	if not self:hasId( item ) then
		local givenId = table.remove( givenIds, 1 )
		self:insertInMap( item, givenId )
	end
	return false
end

local idList = {}

local function getId( self, item )
	if self:hasId( item ) then
		idList[ #idList + 1 ] = self:getId( item )
	end
	return false
end

function GraphIds:getOrderedIds( object, givenIds )
	idList = {}
	self:genericGraphWalk( object, getId, givenIds )
	return idList
end

function GraphIds:giveExternalIds( object, givenIds )
	self:genericGraphWalk( object, giveId, givenIds )
end

function GraphIds:objectId( object, shallow )
	self:genericGraphWalk( object, generateId, shallow )
end

function GraphIds:genericGraphWalk( item, callback, ... )
	local stop = false
	if arg ~= nil then
		stop = callback( self, item, table.unpack(arg) )
	else
		stop = callback( self, item )
	end
	if stop then
		return
	end
	if item.component ~= nil then
		-- is an object
		local ports = self.model:getPorts( item.component )
			
		for i, port in ipairs( ports ) do
			local service = item[port.name]
			if service ~= nil then
				self:genericGraphWalk( service, callback, arg )
			end
		end
	elseif item.interface ~= nil then
		--is an service
		local service = item
		local fields = self.model:getFields( service.interface )
		local kind
		local fieldName
		for i, field in ipairs( fields ) do
			kind = field.type.kind
			fieldName = field.name
			if kind == 'TK_INTERFACE' then
				if service[fieldName] ~= nil then
					self:genericGraphWalk( service[fieldName].provider, callback, arg )
				end
			elseif kind == 'TK_ARRAY' and field.type.elementType.kind == 'TK_INTERFACE' then
				local refs = service[fieldName]
				for i, ref in ipairs( refs ) do
					self:genericGraphWalk( ref.provider, callback, arg )
				end
			end
		end	
	end
end

return GraphIds