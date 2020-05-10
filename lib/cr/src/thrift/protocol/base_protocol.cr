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

# this require is to make generated struct definitions happy
require "set"

module Thrift
    class ProtocolException < Exception

        UNKNOWN = 0
        INVALID_DATA = 1
        NEGATIVE_SIZE = 2
        SIZE_LIMIT = 3
        BAD_VERSION = 4
        NOT_IMPLEMENTED = 5
        DEPTH_LIMIT = 6

        getter type

        def initialize(@type=UNKNOWN, message=nil)
            super(message)
        end
    end

    abstract class BaseProtocol

        getter trans

        def initialize(@trans : BaseTransport)
        end

        abstract def write_message_begin(name : String, type : Thrift::MessageTypes, seqid : Int32)

        abstract def write_message_end

        abstract def write_struct_begin(name : String)

        abstract def write_struct_end

        abstract def write_field_begin(name : String, type : Thrift::Types, id : Int16)

        abstract def write_field_end

        abstract def write_field_stop

        abstract def write_map_begin(ktype : Thrift::Types, vtype : Thrift::Types, size : Int32)

        abstract def write_map_end

        abstract def write_list_begin(etype : Thrift::Types, size : Int32)

        abstract def write_list_end

        abstract def write_set_begin(etype : Thrift::Types, size : Int32)

        abstract def write_set_end

        abstract def write_bool(bool : Bool)

        abstract def write_byte(byte : Int8)

        abstract def write_i16(i16 : Int16)

        abstract def write_i32(i32 : Int32)

        abstract def write_i64(i64 : Int64)

        abstract def write_double(dub : Float64)

        abstract def write_string(str : String)

        abstract def write_binary(buf : Bytes)

        abstract def read_message_begin : Tuple(String, Thrift::MessageTypes, Int32)

        abstract def read_message_end

        abstract def read_struct_begin

        abstract def read_struct_end

        abstract def read_field_begin

        abstract def read_field_end

        abstract def read_map_begin : Tuple(Thrift::Types, Thrift::Types, Int32)

        abstract def read_map_end

        abstract def read_list_begin : Tuple(Thrift::Types, Int32)

        abstract def read_list_end

        abstract def read_set_begin : Tuple(Thrift::Types, Int32)

        abstract def read_set_end

        abstract def read_bool : Bool

        abstract def read_byte : Int8

        abstract def read_i16 : Int16

        abstract def read_i32 : Int32

        abstract def read_i64 : Int64

        abstract def read_double : Float64

        abstract def read_string : String

        abstract def read_binary : Bytes

        def write_field(*args)
            if args.size == 3
                field_info = args[0]
                fid = args[1]
                value = args[2]
            elsif args.size == 4
                field_info = {:name => args[0], :type => args[1]}
                fid = args[2]
                value = args[3]
            else
                raise ArgumentError, "wrong number of arguments (#{args.size} for 3)"
            end

            write_field_begin(field_info[:name], field_info[:type], fid)
            write_type(field_info, value)
            write_field_end
        end

        def write_type(field_info, value)
            if field_info.is_a? Fixnum
                field_info = {:type => field_info}
            end

            case field_info[:type]
            when Types::BOOL
                write_bool(value)
            when Types::BYTE
                write_byte(value)
            when Types::DOUBLE
                write_double(value)
            when Types::I16
                write_i16(value)
            when Types::I32
                write_i32(value)
            when Types::I64
                write_i64(value)
            when Types::STRING
                if field_info[:binary]
                    write_binary(value)
                else
                    write_string(value)
                end
            when Types::STRUCT
                value.write(self)
            else
                raise NotImplementedError
            end
        end

        def read_type(field_info)
            if field_info.is_a? Fixnum
                field_info = {:type => field_info}
            end

            case field_info[:type]
            when Types::BOOL
                read_bool
            when Types::BYTE
                read_byte
            when Types::DOUBLE
                read_double
            when Types::I16
                read_i16
            when Types::I32
                read_i32
            when Types::I64
                read_i64
            when Types::STRING
                if field_info[:binary]
                    read_binary
                else
                    read_string
                end
            else
                raise NotImplementedError
            end
        end

        def skip(type)
            case type
            when Types::BOOL
                read_bool
            when Types::BYTE
                read_byte
            when Types::I16
                read_i16
            when Types::I32
                read_i32
            when Types::I64
                read_i64
            when Types::DOUBLE
                read_double
            when Types::STRING
                read_string
            when Types::STRUCT
                read_struct_begin
                while true
                    name, type, id = read_field_begin
                    break if type == Types::STOP
                    skip(type)
                    read_field_end
                end
                read_struct_end
            when Types::MAP
                ktype, vtype, size = read_map_begin
                size.times do
                    skip(ktype)
                    skip(vtype)
                end
                read_map_end
            when Types::SET
                etype, size = read_set_begin
                size.times do
                    skip(etype)
                end
                read_set_end
            when Types::LIST
                etype, size = read_list_begin
                size.times do
                    skip(etype)
                end
                read_list_end
            else
                raise ProtocolException.new(ProtocolException::INVALID_DATA, "Invalid data")
            end
        end

        def to_s
            "#{trans.to_s}"
        end
    end

    abstract class BaseProtocolFactory
        abstract def get_protocol(trans)
    end
end