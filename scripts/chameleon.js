/**
 * The function calculates the position of the mouse.
 *
 * @param canvas
 * A canvas object.
 *
 * @param evt
 * The event occured.
 *
 * @return
 * Returns an object array containing the x and y coordinates of the mouse
 * pointer at the moment the event occured.
 */
function getMousePos(canvas, evt)
{
	var rect = canvas.getBoundingClientRect();
	return {
		x: evt.clientX - rect.left,
		y: evt.clientY - rect.top
	};
}
/**
 * Finds the path to the document and creates a valid HTTP URI.
 *
 * @param name
 * The name of the wanted file with the relative path to the root.
 *
 * @return
 * A full URL to the wanted file.
 */
function makeURL(name)
{
	var getUrl = window.location;
	var baseUrl = getUrl.protocol + "//" + getUrl.host + "/" + getUrl.pathname.split('/')[1];
	return baseUrl+"/"+name;
}
/**
 * Calculates the position in the image data. With the given \a x and \a y
 * coordinate and the \a width the index position is calculated.
 *
 * @param x
 * The wanted x position in the image
 *
 * @param y
 * The wanted y position in the image
 *
 * @param width
 * the width of the image in pixels.
 *
 * @return
 * An array of index positions. One index for every color channel. The array
 * contains 4 values for red, green, blue and alpha (in this order).
 */
function getColorIndicesForCoord(x, y, width)
{
	var red = y * (width * 4) + x * 4;
	return [red, red + 1, red + 2, red + 3];
}
/**
 * Retrieves the the color at a specific position in the image.
 *
 * @param imgData
 * This is an \b imageData object containing the internal data of an image.
 *
 * @param x
 * The x position in the image.
 *
 * @param y
 * The y position in the image.
 *
 * @param width
 * The width of the image.
 *
 * @return
 * The function returns an array with 4 values. Each value represents one channel
 * of the color of the pixel on \a x and \a y coordinate. The channels are in the
 * order red, green blue and alpha. The range of each channel is from 0 to 255.
 */
function getColor(imgData, x, y, width)
{
	var pixel = [];

	var colorIndices = getColorIndicesForCoord(x, y, width);
	var redIndex = colorIndices[0];
	var greenIndex = colorIndices[1];
	var blueIndex = colorIndices[2];
	var alphaIndex = colorIndices[3];

	pixel.push(imgData.data[redIndex]);
	pixel.push(imgData.data[greenIndex]);
	pixel.push(imgData.data[blueIndex]);
	pixel.push(imgData.data[alphaIndex]);
	return pixel;
}
/**
 * Based on the pixels in the \a basePix, the function decides whether to return
 * the value of \a col1 or \a col2. A red pixel returns the color \a col1 and
 * a green pixel returns the color \a col2. If there is no red and no green
 * pixel, a transparent pixel is returned.
 *
 * @param basePix
 * This is a pixel from a mask containing red and/or green pixels.
 *
 * @param maskPix
 * This is a pixel from a mask containing more or less tranparent pixels. If
 * the alpha channel of \a basePix is 0 (transparent) this pixel is returned.
 *
 * @param col1
 * The first color.
 *
 * @param col2
 * The second color.
 *
 * @return
 * An array containing the color for one pixel.
 */
function baseColor(basePix, maskPix, col1, col2)
{
	var alpha = basePix[3];
	var red = basePix[0];
	var green = basePix[1];

	if (alpha == 0)
		return maskPix;

	if (red)
		return col1;

	if (green)
		return col2;

	return [0, 0, 0, 0];
}
/**
 * Deterines the color to set for a particular pixel in an image and sets all
 * pixels to the correct color or transparaent, if the was no color defined.
 *
 * @param img1
 * The first image. Usualy a mask consisting of only red and/or green pixels
 * and the alpha channel. The blue channel is not used.
 *
 * @param img2
 * The second image. Usualy a mask consisting of white pixels with an alpha
 * channel.
 *
 * @param col1
 * The first color used when \a img1 contains a red pixel.
 *
 * @param col2
 * The second color used when \a img1 contains a green pixel.
 *
 * @param level
 * This is used to display a level value on a \b bargraph.
 * The range is from 0 to 100 (percent).
 *
 * @param dir
 * Defines whether the bargraph is vertical or horizontal. TRUE means vertical.
 * FALSE means horizontal.
 *
 * @return
 * Returns an array of color values. The color values includes the alpha
 * channel and together this is a new image.
 */
function setColor(img1, img2, col1, col2, level, dir)
{
	var x;
	var y;
	var pixel;
	var ctx2;
	var imgData2;

	var width = img1.width;
	var height = img1.height;
	var ctx1 = img1.getContext('2d');
	var imgData1 = ctx1.getImageData(0, 0, width, height);

	if (img2 !== null)
	{
		ctx2 = img2.getContext('2d');
		imgData2 = ctx2.getImageData(0, 0, width, height);
	}

	var pegel = 0;
	var data = [];

	if (dir)
		pegel = height - ~~(height / 100.0 * level);
	else
		pegel = ~~(width / 100.0 * level);

	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			var pix1 = getColor(imgData1, x, y, width);
			var pix2;

			if (img2 !== null)
				pix2 = getColor(imgData2, x, y, width);
			else
				pix2 = [255,255,255,0];

			var base = baseColor(pix1, pix2, col1, col2);
			var alpha = base[3];
			var d = (dir)?y:x;

			if ((dir && pegel <= d) || (!dir && pegel > d))
			{
				if (alpha == 0)
					pixel = pix2;
				else
					pixel = base;
			}
			else
				pixel = [0, 0, 0, 0];

			var colorIndices = getColorIndicesForCoord(x, y, width);
			var redIndex = colorIndices[0];
			var greenIndex = colorIndices[1];
			var blueIndex = colorIndices[2];
			var alphaIndex = colorIndices[3];

			data[redIndex] = pixel[0];
			data[greenIndex] = pixel[1];
			data[blueIndex] = pixel[2];
			data[alphaIndex] = pixel[3];
		}
	}

	return data;
}

/**
 * this function takes 2 URIs. One for a special mask where the green and/or
 * red channel marks whether there should be used col1 or col2.
 * The other mask willbe overlayed on the result from the special mask.
 *
 * @param uriRed
 * This URI should point to a special mask. From this mask only the red and
 * the green channels are used. If the red channel is > 0, than \a col1
 * will be used for that pixel. If the green channel is > 0, than \a col2
 * will be used for that pixel. In every other case the pixel is transparent.
 *
 * @param uriMask
 * This URI points to a mask that is overlayed upon the result of the mask
 * defined with \a uriRed. This mask consists of white pixels with different
 * aplha channel.
 *
 * @param name
 * This is the name of the element where the new \b canvas should be
 * inserted.
 *
 * @param level
 * This is a level in the range of 0 to 100 (percent). Only the given
 * percentage part of mask \a uriRed will be visible. Everything else of
 * this mask will be transparent and therefore invisible.
 *
 * @param width
 * The width of the image.
 *
 * @param height
 * The hight of the image.
 *
 * @param col1
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every red pixel in the mask \a uriRed.
 *
 * @param col2
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every green pixel in the mask \a uriRed.
 *
 * @param dir
 * Boolean option to set the direction of the bargraph. FALSE means vertical
 * and TRUE means horizontal.
 */
async function drawBargraph(uriRed, uriMask, name, level, width, height, col1, col2, dir, feedback = false)
{
	if (width <= 0 || height <= 0 || uriRed.length == 0 || uriMask.length == 0)
	{
		debug("drawBargraph: name="+name+", uriRed="+uriRed+", uriMask="+uriMask+", width="+width+", height="+height+", level="+level);
		return;
	}

	var readyPic1 = false;
	var readyPic2 = false;
	var canvas1 = document.createElement('canvas');
	var canvas2 = document.createElement('canvas');
	var canvas3 = document.createElement('canvas');

	if (canvas1.getContext && canvas2.getContext && canvas3.getContext)
	{
		var ctx1 = canvas1.getContext('2d');
		var ctx2 = canvas2.getContext('2d');
		var ctx3 = canvas3.getContext('2d');

		var img = new Image();
		var imgCham = new Image();
		imgCham.src = uriMask;
		imgCham.setAttribute('crossOrigin', '');
		img.src = uriRed;
		img.setAttribute('crossOrigin', '');

		canvas1.width = width;
		canvas1.height = height;
		canvas2.width = width;
		canvas2.height = height;
		canvas3.width = width;
		canvas3.height = height;

		if (!img.complete)
		{
			img.onload = function()
			{
				ctx1.drawImage(img, 0, 0);
				readyPic1 = true;
			}
		}
		else
		{
			ctx1.drawImage(img, 0, 0);
			readyPic1 = true;
		}

		if (!imgCham.complete)
		{
			imgCham.onload = function()
			{
				ctx2.drawImage(imgCham, 0, 0);
				readyPic2 = true;
			}
		}
		else
		{
			ctx2.drawImage(imgCham, 0, 0);
			readyPic2 = true;
		}

		var cnt = 0;

		while((!readyPic1 || !readyPic2) && cnt < 20)
		{
			await new Promise(r => setTimeout(r, 200));
			cnt++;
		}

		if (!readyPic1)
		{
			errlog("drawBargraph: WARNING: "+uriRed+" not loaded!");
			ctx1.drawImage(img, 0, 0);
		}

		if (!readyPic2)
		{
			errlog("drawBargraph: WARNING: "+uriMask+" not loaded!");
			ctx2.drawImage(imgCham, 0, 0);
		}

		var data = setColor(canvas1, canvas2, col1, col2, level, dir);
		var imgData = ctx1.getImageData(0, 0, width, height);
		imgData.data.set(data);
		ctx1.putImageData(imgData, 0, 0);

		ctx3.drawImage(canvas1, 0, 0);
		ctx3.globalCompositeOperation = "source-atop";
		ctx3.drawImage(canvas2, 0, 0);

		canvas3.id = name+"_canvas";
		canvas3.className = name+"_canvas";
		var div = document.getElementById(name);

		if (div === null)
			return;

		try
		{
			div.replaceChild(canvas3, document.getElementById(name+"_canvas"));
		}
		catch(e)
		{
			div.appendChild(canvas3);
			div.insertBefore(canvas3, div.firstChild);
		}

		if (feedback)
        {
            canvas3.addEventListener('click', function(evt)
            {
                var mousePos = getMousePos(canvas3, evt);
                var lev = 0;

                if (dir)
                    lev = 100 - ~~(100.0 / height * mousePos.y);
                else
                    lev = ~~(100.0 / width * mousePos.x);

                drawBargraph(uriRed, uriMask, name, lev, width, height, col1, col2, dir);
            }, false);
        }
	}
	else
	{
		errlog("drawBargraph: Error getting context for canvas "+name+"!");
		return false;
	}

	return true;
}
/**
 * this function takes 2 URIs. One for a special mask where the green and/or
 * red channel marks whether there should be used col1 or col2.
 * The other mask willbe overlayed on the result from the special mask.
 *
 * @param uriRed
 * This URI should point to a special mask. From this mask only the red and
 * the green channels are used. If the red channel is > 0, than \a col1
 * will be used for that pixel. If the green channel is > 0, than \a col2
 * will be used for that pixel. In every other case the pixel is transparent.
 *
 * @param uriMask
 * This URI points to a mask that is overlayed upon the result of the mask
 * defined with \a uriRed. This mask consists of white pixels with different
 * aplha channel.
 *
 * @param name
 * This is the name of the element where the new \b canvas should be
 * inserted.
 *
 * @param width
 * The width of the image.
 *
 * @param height
 * The hight of the image.
 *
 * @param col1
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every red pixel in the mask \a uriRed.
 *
 * @param col2
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every green pixel in the mask \a uriRed.
 */
async function drawButton(uriRed, uriMask, name, width, height, col1, col2)
{
	var readyPic1 = false;
	var readyPic2 = false;
	var canvas1 = document.createElement('canvas');
	var canvas2 = document.createElement('canvas');
	var canvas3 = document.createElement('canvas');

	if (canvas1.getContext && canvas2.getContext && canvas3.getContext)
	{
		var ctx1 = canvas1.getContext('2d');
		var ctx2 = canvas2.getContext('2d');
		var ctx3 = canvas3.getContext('2d');

		var img = new Image();
		var imgCham = new Image();
		imgCham.src = encodeURI(uriMask);
		imgCham.setAttribute('crossOrigin', '');
		img.src = encodeURI(uriRed);
		img.setAttribute('crossOrigin', '');

		canvas1.width = width;
		canvas1.height = height;
		canvas2.width = width;
		canvas2.height = height;
		canvas3.width = width;
		canvas3.height = height;

		if (!img.complete)
		{
			img.onload = function()
			{
				ctx1.drawImage(img, 0, 0);
				readyPic1 = true;
			}
		}
		else
		{
			ctx1.drawImage(img, 0, 0);
			readyPic1 = true;
		}

		if (!imgCham.complete)
		{
			imgCham.onload = function()
			{
				ctx2.drawImage(imgCham, 0, 0);
				readyPic2 = true;
			}
		}
		else
		{
			ctx2.drawImage(imgCham, 0, 0);
			readyPic2 = true;
		}

		var cnt = 0;

		while((!readyPic1 || !readyPic2) && cnt < 20)
		{
			await new Promise(r => setTimeout(r, 200));
			cnt++;
		}

		if (!readyPic1)
		{
			errlog("drawButton: WARNING: "+uriRed+" not loaded!");
			ctx1.drawImage(img, 0, 0);
		}

		if (!readyPic2)
		{
			errlog("drawButton: WARNING: "+uriMask+" not loaded!");
			ctx2.drawImage(imgCham, 0, 0);
		}

		var data = setColor(canvas1, canvas2, col1, col2, 100, false);
		var imgData = ctx1.getImageData(0, 0, width, height);
		imgData.data.set(data);
		ctx1.putImageData(imgData, 0, 0);

		ctx3.drawImage(canvas1, 0, 0);
		ctx3.globalCompositeOperation = "source-atop";
		ctx3.drawImage(canvas2, 0, 0);

		var div = document.getElementById(name);
		canvas3.id = name+"_canvas";
		canvas3.className = name+"_canvas";

		try
		{
			div.replaceChild(canvas3, document.getElementById(name+"_canvas"));
		}
		catch(e)
		{
			div.appendChild(canvas3);
			div.insertBefore(canvas3, div.firstChild);
		}
	}
	else
	{
		errlog("drawButton: Error getting context for canvas "+name+"!");
		return false;
	}

	return true;
}
/**
 * This function takes 1 URI for a special mask where the green and/or
 * red channel marks whether there should be used col1 or col2. At positions
 * Where no red and no green color is defined, a transparent pixel is set.
 *
 * @param uriRed
 * This URI should point to a special mask. From this mask only the red and
 * the green channels are used. If the red channel is > 0, than \a col1
 * will be used for that pixel. If the green channel is > 0, than \a col2
 * will be used for that pixel. In every other case the pixel is transparent.
 *
 * @param name
 * This is the name of the element where the new \b canvas should be
 * inserted.
 *
 * @param width
 * The width of the image.
 *
 * @param height
 * The hight of the image.
 *
 * @param col1
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every red pixel in the mask \a uriRed.
 *
 * @param col2
 * This color must be an array with the values for red, green, blue and
 * alpha channels. Every color and the alpha channel must be in the range
 * from 0 to 255.
 * This color is taken for every green pixel in the mask \a uriRed.
 */
async function drawArea(uriRed, name, width, height, col1, col2)
{
	var readyPic1 = false;
	var canvas1 = document.createElement('canvas');
	var canvas2 = document.createElement('canvas');

	if (canvas1.getContext && canvas2.getContext)
	{
		var ctx1 = canvas1.getContext('2d');
		var ctx2 = canvas2.getContext('2d');

		var img = new Image();
		img.src = encodeURI(uriRed);
		img.setAttribute('crossOrigin', '');

		canvas1.width = width;
		canvas1.height = height;
		canvas2.width = width;
		canvas2.height = height;

		if (!img.complete)
		{
			img.onload = function()
			{
				ctx1.drawImage(img, 0, 0);
				readyPic1 = true;
			}
		}
		else
		{
			ctx1.drawImage(img, 0, 0);
			readyPic1 = true;
		}

		var cnt = 0;

		while(!readyPic1 && cnt < 20)
		{
			await new Promise(r => setTimeout(r, 200));
			cnt++;
		}

		if (!readyPic1)
		{
			errlog("drawArea: WARNING: "+uriRed+" not loaded!");
			ctx1.drawImage(img, 0, 0);
		}

		var data = setColor(canvas1, null, col1, col2, 100, false);
		var imgData = ctx1.getImageData(0, 0, width, height);
		imgData.data.set(data);
		ctx1.putImageData(imgData, 0, 0);
		ctx2.drawImage(canvas1, 0, 0);

		var div = document.getElementById(name);
		canvas2.id = name+"_canvas";
		canvas2.className = name+"_canvas";

		try
		{
			div.replaceChild(canvas2, document.getElementById(name+"_canvas"));
		}
		catch(e)
		{
			div.appendChild(canvas2);
			div.insertBefore(canvas2, div.firstChild);
		}
	}
	else
	{
		errlog("drawArea: Error getting context for canvas "+name+"!");
		return false;
	}

	return true;
}
