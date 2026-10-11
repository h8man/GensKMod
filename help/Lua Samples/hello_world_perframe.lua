frame = 0

while true do
	frame = frame + 1
	kmod.log("hello world #" .. frame)
	coroutine.yield()
end
