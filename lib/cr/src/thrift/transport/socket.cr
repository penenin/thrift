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

require "socket"

module Thrift
    class Socket < BaseTransport
        def initialize(@host="localhost", @port=9090, @timeout=0)
            @desc = "#{host}:#{port}"
            @handle = nil
        end

        property handle, timeout

        def open
            begin
                @handle = TCPSocket.new(@host, @port)
            rescue e : Exception
                raise TransportException.new(TransportException::NOT_OPEN, "Could not connect to #{@desc}: #{e.message}")
            end
        end
    
        def open?
            !@handle.nil? && !@handle.closed?
        end

        def write(str)
            handle = @handle
            begin
                handle.write_bytes(str, IO::ByteFormat::BigEndian) if handle
            rescue e : TransportException
                raise e
            rescue e : Exception
                handle.close if handle
                @handle = nil
                raise TransportException.new(TransportException::NOT_OPEN, e.message)
            end
        end

        def write(str : Bytes)
            handle = @handle
            begin
                handle.write(str) if handle
            rescue e : TransportException
                raise e
            rescue e : Exception
                handle.close if handle
                @handle = nil
                raise TransportException.new(TransportException::NOT_OPEN, e.message)
            end
        end

        def read(sz)
            handle = @handle
            begin
                if handle
                    data = Bytes.new(sz)
                    handle.read(data)
                end
            rescue e : TransportException
                raise e
            rescue e : Exception
                handle.close unless handle.nil? || handle.closed?
                @handle = nil
                raise TransportException.new(TransportException::NOT_OPEN, e.message)
            end
            if data.nil? || data.size == 0
                raise TransportException.new(TransportException::UNKNOWN, "Socket: Could not read #{sz} bytes from #{@desc}")
            end
            data
        end

        def close
            @handle.close unless @handle.nil? || @handle.closed?
            @handle = nil
        end

        def flush
            @handle.flush
        end

        def to_s
            "socket(#{@host}:#{@port})"
        end
    end
end
