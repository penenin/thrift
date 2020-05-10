# 
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
# 


require "./thrift/exceptions"
require "./thrift/types"
require "./thrift/protocol/base_protocol"
require "./thrift/protocol/binary_protocol"
require "./thrift/server/base_server.cr"
require "./thrift/server/simple_server.cr"
require "./thrift/transport/base_server_transport"
require "./thrift/transport/base_transport"
require "./thrift/transport/buffered_transport"
require "./thrift/transport/memory_buffer_transport"
require "./thrift/transport/server_socket"
require "./thrift/transport/socket"

# TODO: Write documentation for `Thrift`
module Thrift
  VERSION = "0.14.0"

  # TODO: Put your code here
end
