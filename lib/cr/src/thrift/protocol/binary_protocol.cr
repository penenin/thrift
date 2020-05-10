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

require "io"

module Thrift
    class BinaryProtocol < BaseProtocol
        VERSION_MASK = 0xffff0000
        VERSION_1 = 0x8001_u16
        TYPE_MASK = 0x000000ff

        getter strict_read, strict_write

        def initialize(trans, @strict_read = true, @strict_write = true)
            super(trans)
            @rbuf = Bytes.new(8)
        end

        def write_message_begin(name : String, type : Thrift::MessageTypes, seqid : Int32)
            if strict_write
                write_i16(VERSION_1)
                write_i16(type.value.to_i16)
                write_string(name)
                write_i32(seqid)
            else
                write_string(name)
                write_byte(type.value.to_i8)
                write_i32(seqid)
            end
        end

        def write_message_end
            nil
        end

        def write_struct_begin(name)
            nil
        end

        def write_struct_end
            nil
        end

        def write_field_begin(name : String, type : Thrift::Types, id : Int16)
            write_byte(type.value)
            write_i16(id)
        end

        def write_field_end
            nil
        end

        def write_field_stop
            write_byte(Thrift::Types::STOP.value)
        end

        def write_map_begin(ktype : Thrift::Types, vtype : Thrift::Types, size : Int32)
            write_byte(ktype.value)
            write_byte(vtype.value)
            write_i32(size)
        end

        def write_map_end
            nil
        end

        def write_list_begin(etype : Thrift::Types, size : Int32)
            write_byte(etype.value)
            write_i32(size)
        end

        def write_list_end
            nil
        end

        def write_set_begin(etype, size)
            write_byte(etype.value)
            write_i32(size)
        end

        def write_set_end
            nil
        end

        def write_bool(bool : Bool)
            write_byte(bool ? 1_i8 : 0_i8)
        end

        def write_byte(byte : Int8)
            trans.write(byte)
        end

        def write_i16(i16 : Int16 | UInt16)
            trans.write(i16)
        end

        def write_i32(i32 : Int32)
            trans.write(i32)
        end

        def write_i64(i64 : Int64)
            trans.write(i64)
        end

        def write_double(dub : Float64)
            trans.write(dub)
        end

        def write_string(str : String)
            buf = str.to_slice
            write_binary(buf)
        end

        def write_binary(buf : Bytes)
            write_i32(buf.size)
            trans.write(buf)
        end

        def read_message_begin : Tuple(String, Thrift::MessageTypes, Int32)
            version = read_i32
            if version < 0
                if version & VERSION_MASK != VERSION_1
                    raise ProtocolException.new(ProtocolException::BAD_VERSION, "Missing version identifier")
                end
                type = version & TYPE_MASK
                name = read_string
                seqid = read_i32
                {name, type, seqid}
            else
                if strict_read
                    raise ProtocolException.new(ProtocolException::BAD_VERSION, "No version identifier, old protocol client?")
                end
                name = trans.read_all(version)
                type = read_byte
                seqid = read_i32
                {name, type, seqid}
            end
        end

        def read_message_end
            nil
        end
        
        def read_struct_begin
            nil
        end
        
        def read_struct_end
            nil
        end

        def read_field_begin
            type = read_byte
            if type == Types::STOP
                {nil, Thrift::Types.new(type), 0_i16}
            else
                id = read_i16
                {nil, Thrift::Types.new(type), id}
            end
        end

        def read_field_end
            nil
        end

        def read_map_begin : Tuple(Thrift::Types, Thrift::Types, Int32)
            ktype = read_byte
            vtype = read_byte
            size = read_i32
            {Thrift::Types.new(ktype), Thrift::Types.new(vtype), size}
        end

        def read_map_end
            nil
        end

        def read_list_begin : Tuple(Thrift::Types, Int32)
            etype = read_byte
            size = read_i32
            {Thrift::Types.new(etype), size}
        end

        def read_list_end
            nil
        end

        def read_set_begin : Tuple(Thrift::Types, Int32)
            etype = read_byte
            size = read_i32
            {Thrift::Types.new(etype), size}
        end

        def read_set_end
            nil
        end

        def read_bool : Bool
            byte = read_byte
            byte != 0
        end

        def read_byte : Int8
            rbuf = trans.read(1)
            io = IO::Memory.new rbuf
            byte = io.read_byte
            if byte
                return byte.to_i8!
            else
                return 0_i8
            end
        end

        def read_i16 : Int16
            rbuf = trans.read(2)
            io = IO::Memory.new rbuf
            io.read_bytes(Int16, IO::ByteFormat::BigEndian)
        end

        def read_i32 : Int32
            rbuf = trans.read(4)
            io = IO::Memory.new rbuf
            io.read_bytes(Int32, IO::ByteFormat::BigEndian)
        end

        def read_i64 : Int64
            rbuf = trans.read(8)
            io = IO::Memory.new rbuf
            io.read_bytes(Int64, IO::ByteFormat::BigEndian)
        end

        def read_double : Float64
            rbuf = trans.read(4)
            io = IO::Memory.new rbuf
            io.read_bytes(Float64, IO::ByteFormat::BigEndian)
        end

        def read_string : String
            buffer = read_binary
            String.new(buffer)
        end

        def read_binary : Bytes
            size = read_i32
            trans.read(size)
        end

        def to_s
            "binary(#{super.to_s})"
        end
    end

    class BinaryProtocolFactory < BaseProtocolFactory
        def get_protocol(trans)
            return Thrift::BinaryProtocol.new(trans)
        end

        def to_s
            "binary"
        end
    end
end