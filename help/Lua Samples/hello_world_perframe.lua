frame = 0

while not kmod.should_stop() do
	frame = frame + 1
	kmod.log("hello world #" .. frame)
	coroutine.yield()
end
