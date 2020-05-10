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
    class BufferedTransport < BaseTransport
        def initialize(@transport)
            @rbuf = IO::Memory.new
            @wbuf = IO::Memory.new
            @index = 0
        end

        def open?
            @transport.open?
        end

        def open
            @transport.open
        end

        def close
            flush
            @transport.close
        end

        def read(sz)
            ret = Bytes.new(sz)
            read_into_buffer(ret, sz)
            ret
        end

        def read_byte
            byte = @rbuf.read_byte
            if !byte.nil?
                return byte.to_i8!
            else
                return 0_i8
            end
        end

        def read_into_buffer(buffer, size)
            @rbuf.read(buffer)
        end

        def write(byte : Int8)
            @wbuf.write_byte(byte.to_u8)
        end

        def write(value : Int16 | Int32 | Int64 | UInt16 | UInt32 | UInt64 | Float64)
            @wbuf.write_bytes(value, IO::ByteFormat::BigEndian)
        end

        def write(wbuf : Bytes)
            @wbuf.write(wbuf)
        end

        def flush
            unless @wbuf.empty?
                @transport.write(@wbuf)
                @wbuf = IO::Memory.new
            end

            @transport.flush
        end

        def to_s
            "buffered(#{@transport.to_s})"
        end
    end

    class BufferedTransportFactory < BaseTransportFactory
        def get_transport(transport)
            BufferedTransport.new(transport)
        end
    end
end