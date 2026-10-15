-- VRAM scramber's goal is to collect Plane information,
-- and serialize it with appropriate VScroll and HScroll shifts
-- essentially to serialize lvl tiles position into 565 texture
-- or bin ttc format

-- TTC v1 header: signature "TTC1", then big-endian 16-bit width (SX), height (SY), and empty-cell/mask key value (MK); 16 bytes total.
-- Start of the file will be: TTC1SX##SY##MK## 

--unsigned short to signed short (16 bits)
function us2si(ss)
  if (ss & 0x8000) ~= 0 then --negative
	ss = ss - 0x10000;
  end
  return ss;
end

function writeBmp565Header(file_stream, width, height)
  local rowPadding = (4 - (width * 2) % 4) % 4
  local imageSize = (width * 2 + rowPadding) * height
  local pixelOffset = 66

  file_stream:write(string.pack("<c2I4I2I2I4", "BM", pixelOffset + imageSize, 0, 0, pixelOffset))
  file_stream:write(string.pack("<I4i4i4I2I2I4I4i4i4I4I4",
	40, width, height, 1, 16, 3, imageSize, 2835, 2835, 0, 0))
  file_stream:write(string.pack("<I4I4I4", 0xF800, 0x07E0, 0x001F))
  return rowPadding
end

function writeTtcHeader(file_stream, width, height, magicNumber)
  file_stream:write(table.concat({
	"TTC1",
	"SX", string.pack(">I2", width),
	"SY", string.pack(">I2", height),
	"MK", string.pack(">I2", magicNumber)
  }))
end

function writeTtc(texture, Nx, Ny, file_stream)
	local batchSize = 2048
  local cellCount = Nx * Ny

  writeTtcHeader(file_stream, Nx, Ny, MagicNumber)

  for startIndex=1, cellCount, batchSize do
	local count = math.min(batchSize, cellCount - startIndex + 1)
	local values = {}
	for offset=0, count - 1 do
	  values[offset + 1] = texture[startIndex + offset] or MagicNumber
	end
	file_stream:write(string.pack(">" .. string.rep("I2", count), table.unpack(values, 1, count)))
  end
end

function writeBmp565(texture, width, height, file_stream)
	local batchSize = 2048
  local rowPadding = writeBmp565Header(file_stream, width, height)

	for outputRow=1, height do
	local textureRow = height - outputRow + 1
	local rowStart = (textureRow - 1) * width + 1
	local rowEnd = rowStart + width - 1

	for startIndex=rowStart, rowEnd, batchSize do
	  local count = math.min(batchSize, rowEnd - startIndex + 1)
	  local values = {}
	  for offset=0, count - 1 do
		values[offset + 1] = texture[startIndex + offset] or MagicNumber
	  end
	  file_stream:write(string.pack("<" .. string.rep("I2", count), table.unpack(values, 1, count)))
	end

	if rowPadding > 0 then
	  file_stream:write(string.rep("\0", rowPadding))
	end
  end
end

function exit( )
  -- cleanup code (if any) goes here
	local extension = bWriteBmp and ".bmp" or ".ttc"
  local writeTexture = bWriteBmp and writeBmp565 or writeTtc
  local f1 = io.open(Path .. "PlaneA" .. extension, "wb")
	writeTexture(textureA, NSize, NSize, f1)
  f1:close();
  local f2 = io.open(Path .. "PlaneB" .. extension, "wb")
  writeTexture(textureB, NSize, NSize, f2)
  f2:close();
end 

-- initialize
kmod.log("VRAM Scramble - START")

NSize=300; -- output texture size (NxN) in tiles, user defined
MagicNumber = 0x4fb; -- magic number/empty value. User defined 0x0000 .. 0xffff
Path = ".\\"
bWriteBmp = true; -- write 565 BMP files instead of TTC files

kmod.log(string.format("NSize=%d MagicNumber=%04X Path=%s bWriteBmp=%s", NSize, MagicNumber, Path, tostring(bWriteBmp)))
kmod.log("VRAM Scramble - Warming up...")

textureA = {}
textureB = {}
textureACellCount = 0
textureBCellCount = 0
textureCellTotal = NSize * NSize
  
kmod.log("VRAM Scramble - PREFILL")

magicNumberFoundA = false
magicNumberFoundB = false

while not kmod.should_stop() do
  -- put any code you want to run after each frame here
  -- (such as getting the last frame's input or reading from memory)

  local reg1 = vdp.readregister(1)
  local reg2 = vdp.readregister(2)
  local reg4 = vdp.readregister(4)
  local reg12 = vdp.readregister(12)
  local reg16 = vdp.readregister(16)
  local PlaneA = (reg2 & 0x38) << 10 -- plane A base address in VRAM
  local PlaneB = (reg4 & 0x07) << 13 -- plane B base address in VRAM
  local hNTSC40 = 32 + (reg12 & 0x01) * 8 -- horizontal visible tiles in NTSC 40-column mode
  local vNTSC40 = 28 + ((reg1 >> 3) & 0x01) * 2 -- vertical visible tiles in NTSC 40-column mode
  local hPlane = 32 + (reg16 & 0x03) * 32 -- horizontal tiles in plane
  local vPlane = 32 + ((reg16 >> 4) & 0x03) * 32 -- vertical tiles in plane

  uHS = vdp.readwordvram(0x0000); --usigned horizontal scroll value
  uVS = vdp.readwordvsram(0x0000); --usigned vertical scroll value

  iHS = -us2si(uHS);
  iVS = us2si(uVS);
  
  --scroll to tiles 8x8 on Plane coordinates
  HS8 = math.floor(iHS/8) % hPlane;
  VS8 = math.floor(iVS/8) % vPlane;
  
  --plane scroll to output tile global coordinates in texture
  tile_x = math.floor(iHS/8)+math.floor(NSize/2);
  tile_y = math.floor(iVS/8)+math.floor(NSize/2);
  kmod.clearlog()

  kmod.log(string.format("VDP regs: R1=%02X R2=%02X R4=%02X R12=%02X R16=%02X", reg1, reg2, reg4, reg12, reg16))
  kmod.log(string.format("Layout: PlaneA=%04X PlaneB=%04X | Visible=%dx%d Plane=%dx%d", PlaneA, PlaneB, hNTSC40, vNTSC40, hPlane, vPlane))
  kmod.log(string.format("Scroll: raw HS=%04X VS=%04X | signed HS=%d VS=%d | tiles HS8=%d VS8=%d | origin=(%d, %d)", uHS, uVS, iHS, iVS, HS8, VS8, tile_x, tile_y))
  kmod.log(string.format("Texture fill: A=%d/%d (%.1f%%) B=%d/%d (%.1f%%)", textureACellCount, textureCellTotal, textureACellCount * 100 / textureCellTotal, textureBCellCount, textureCellTotal, textureBCellCount * 100 / textureCellTotal))

  if magicNumberFoundA then
	kmod.log(string.format("WARNING: MagicNumber 0x%04X was read from Plane A VRAM and stored in textureA", MagicNumber))
  end
  if magicNumberFoundB then
	kmod.log(string.format("WARNING: MagicNumber 0x%04X was read from Plane B VRAM and stored in textureB", MagicNumber))
  end


  for i=0, (hNTSC40-1) , 1 do
	local outputX = tile_x + i
	local h_offset = (i+HS8)%hPlane

    for j=0, (vNTSC40-1), 1 do
	  local outputY = tile_y + j
	  if outputX >= 1 and outputX <= NSize and outputY >= 1 and outputY <= NSize then
		local textureIndex = (outputY - 1) * NSize + outputX
		local v_offset = (j+VS8)%vPlane
		local tileA = vdp.readwordvram(PlaneA+2*h_offset+2*v_offset*hPlane)
		local tileB = vdp.readwordvram(PlaneB+2*h_offset+2*v_offset*hPlane)
		if textureA[textureIndex] == nil then textureACellCount = textureACellCount + 1 end
		if textureB[textureIndex] == nil then textureBCellCount = textureBCellCount + 1 end
		textureA[textureIndex] = tileA
		textureB[textureIndex] = tileB

		if magicNumberFoundA == false and tileA == MagicNumber then magicNumberFoundA = true end
		if magicNumberFoundB == false and tileB == MagicNumber then magicNumberFoundB = true end
	  end
	end
  end

  coroutine.yield()
end
kmod.log("VRAM Scramble - EXIT")

exit()

kmod.log("VRAM Scramble - DONE")