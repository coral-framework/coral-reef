
function test()
local transportObj = co.new "mockReef.Transport"
local transport = transportObj.transport

local serverANodeObj = co.new "reef.rpc.Node"
serverANodeObj.transport = transport

local cliNodeObj = co.new "reef.rpc.Node"
cliNodeObj.transport = transport

local serverBNodeObj = co.new "reef.rpc.Node"
serverBNodeObj.transport = transport

local serverA = serverANodeObj.node;
local client = cliNodeObj.node
local serverB = serverBNodeObj.node
transportObj.node = serverA
transportObj.node = client
transportObj.node = serverB

serverA:start( "addressA", "addressA" )   

client:start( "addressClient", "addressClient" )

serverB:start( "addressB", "addressB" )


local instanceInA = client:newRemoteInstance( "moduleA.TestComponent",	"addressA" )
local refTypesServiceInA = instanceInA.reference
local simpleTypesServiceInA = instanceInA.simple

local instanceInB = client:newRemoteInstance( "moduleA.TestComponent", "addressB" )
local simpleTypesServiceInB = instanceInB.simple

local instanceLocal = co.new "moduleA.TestComponent"
local simpleTypesServiceLocal = instanceLocal.simple

EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceInA, 3 ), 4 )
EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceInA, 15, 5 ), 3 )
EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceInA, "aaa", "bbb" ), "aaabbb" )

EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceLocal, 3 ), 4 )
EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceLocal, 15, 5 ), 3 )
EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceLocal, "aaa", "bbb" ), "aaabbb" )

EXPECT_EQ( refTypesServiceInA:callIncrementInt( simpleTypesServiceInB, 3 ), 4 )
EXPECT_EQ( refTypesServiceInA:callDivideDouble( simpleTypesServiceInB, 15, 5 ), 3 )
EXPECT_EQ( refTypesServiceInA:concatenateString( simpleTypesServiceInB, "aaa", "bbb" ), "aaabbb" )

end
