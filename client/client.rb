require 'socket'

start_time = Process.clock_gettime(Process::CLOCK_MONOTONIC)

socket = TCPSocket.new 'localhost', 6969
socket.write("#{ARGV[0]}\n")
socket.each_line do |line|
    #puts line
end
socket.close

end_time = Process.clock_gettime(Process::CLOCK_MONOTONIC)
puts "File: #{ARGV[0]}\t Elapsed: #{(end_time-start_time).round(3)}\n"