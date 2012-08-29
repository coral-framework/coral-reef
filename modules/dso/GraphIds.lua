GraphIds = {}
local GraphIdsMT = { __index = GraphIds }

function GraphIds:new( space )
	self = {}
	self.space = space
	
	self.model = space.universe.model
	
	self.objectIdMap = {}
	self.idObjectMap = {}
	self.currentId = 1
	setmetatable( self, GraphIdsMT )
	self:objectId( space.rootObject )
	return self;
end

function GraphIds:insertInMap( service )
	self.objectIdMap[service] = self.currentId
	self.idObjectMap[self.currentId] = service
	self.currentId = self.currentId + 1
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

function GraphIds:objectId( object )
	if not self:hasId( object ) then
		self:insertInMap( object )
		local ports = self.model:getPorts( object.component )
		
		for i, port in ipairs( ports ) do
			local service = object[port.name]
			if service ~= nil then
				self:serviceId( service )
			end
		end
	end
end

function GraphIds:serviceId( service )
	if not self:hasId( service ) then
		self:insertInMap( service )
		local fields = self.model:getFields( service.interface )
		local kind
		local fieldName
		for i, field in ipairs( fields ) do
			kind = field.type.kind
			fieldName = field.name
			if kind == 'TK_INTERFACE' then
				if service[fieldName] ~= nil then
					self:objectId( service[fieldName].provider )
				end
			elseif kind == 'TK_ARRAY' and field.type.elementType.kind == 'TK_INTERFACE' then
				local refs = service[fieldName]
				for i, ref in ipairs( refs ) do
					self:objectId( ref.provider )
				end
			end
		end
	end
end

return GraphIds