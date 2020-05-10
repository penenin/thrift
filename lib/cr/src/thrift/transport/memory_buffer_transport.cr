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

module Thrift
    class MemoryBufferTransport < BaseTransport
        
        def initialize()
            @io = IO::Memory.new
            @index = 0
        end

        def open?
            !@io.closed?
        end

        def open
        end

        def close
            @io.close
        end

        def peek
            @io.peek
        end

        def reset_buffer()
            @io.rewind
            @index = 0
        end

        def available
            @io.size - @index
        end

        def read(len)
            data = Bytes.new(len)
            @index += read_into_buffer(data)
            data
        end

        def read_byte : Int8          
            @index += 1
            byte = @io.read_byte
            if !byte.nil?
                return byte.to_i8!
            else
                return 0_i8
            end
        end

        def read_into_buffer(buffer)
            @io.read(buffer)
        end

        def write(byte : Int8)
            @io.write_byte(byte.to_u8)
        end

        def write(value : Int16 | Int32 | Int64 | UInt16 | UInt32 | UInt64 | Float64)
            @io.write_bytes(value, IO::ByteFormat::BigEndian)
        end

        def write(wbuf : Bytes)
            @io.write(wbuf)
        end

        def flush
        end

        def to_s
            "memory"
        end
    end
end