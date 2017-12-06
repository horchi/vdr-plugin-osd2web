//---------------------------------------------------------------------------------------
// XSnow for JavaScript - von Mag. Dr. Nikolaus Klepp - dr.klepp@gmx.at - www.klepp.info
//---------------------------------------------------------------------------------------
/*  jsSnow
    Copyright (C) 2002 Mag. Dr. Nikolaus Klepp <dr.klepp@gmx.at>
	Copyright (C) 2002 INOUE Hiroyuki <dombly@kc4.so-net.ne.jp>
	Copyright (C) 2002 Heiko Feldker <hfeldker@web.de>
	Release Id: 0.3n1+dombly1

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
//---------------------------------------------------------------------------------------

import snow0 from "./snow0.gif";
import snow1 from "./snow1.gif";
import snow2 from "./snow2.gif";
import snow3 from "./snow3.gif";
import snow4 from "./snow4.gif";
import snow5 from "./snow5.gif";
import snow6 from "./snow6.gif";
import sleigh from "./sleigh3.gif"

	//window.onerror = null;

	var ns6 = (!document.all && document.getElementById);
	var ie4 = (document.all);
	var ns4 = (document.layers);
	var BV  = (parseFloat(navigator.appVersion.indexOf("MSIE")>0 ?
				navigator.appVersion.split(";")[1].substr(6)	 :
				navigator.appVersion));
	var op  = (navigator.userAgent.indexOf('Opera')!=-1 && BV>=4);

	var pageWidth  = 0;						// page dimension & visible offset
	var pageHeight = 0;
	var pageOffX   = 0;
	var pageOffY   = 0;


	// <---- Customizable part ----
	var infoLayer  = false;					// set to false if you don't need f/s display

	var flakes = 25;						// total number of snowflakes
	var santa_mass = 5;                     // santa's effective mass during storms
											//   specified as the ratio to snow flakes
											//   exept 0, which means santa won't be pushed by storms
	var santa_speed_PperS =  20;			// santa speed in pixel/second
	var flake_speed_PperS =  30;			// flake speed in pixel/second
	var storm_speed_PperS = 300;			// storm speed in pixel/second

	var santa_init_x	 = -256;			// santa's initial X-position
	var santa_direction  = 0;				// santa's movement direction in degree
											//   (between [0:360]; default is (1,0), 90 is to go up )
	var santa_appearance = 0.75;			// probability between [0:1] for santa to be shown

	var flake_TX          = 1.0;            // max. sec. of flake's constant X-movement on fluttering
	var flake_XperY		  = 2.0;			// fluttering movement's max. vx/vy ratio
	var santa_TY          = 0.5;            // max. sec. of santa's constant Y-movement in his rugged movement
	var santa_YperX       = 0.5;            // santa's rugged movement's max. vy/vx ratio

	var storm_duration_S  = 10.0;			// storm duration in seconds - watch out: about 1-2 seconds for deceleration
	var storm_lag_S       = 60.0;			// no-storm in seconds
	var storm_YperX       = 1/3.0;			// storm's max. vy/vx ratio

	var disappear_margin = 32;				// pixels from each border where objects disappear
	// ---- Customizable part ---->


	var refresh_FperS = 5;//20;					// initial frames/second, recalculated.
	var refresh 	  = 1000/refresh_FperS;	// ms/frame

	var santa_speed 	= 0;				// santa speed in pixel/frame
	var flake_speed 	= 0;				// flake speed in pixel/frame
	var storm_speed 	= 0;				// storm speed in pixel/frame
	var storm_YperX_current = storm_YperX;  // storm direction varies each time
	var storm_v_sin     = 0;                // storm speed's sine
	var storm_v_cos     = 1;                // storm speed's cosine
	var storm_direction = 0;				// storm X-direction, -1/0=quiet/+1
	var storm_id    	= 0;				// ID of storm timer

	var storm_blowing	= 1;				// start with storm=ON

	var santa;
	var santaX	= 0;			// X-position of santa
	var santaY	= 0;			// Y-position of santa
	var santaSY = 0;			// frames till Y-movement changes
	var santaVY = 0;			// variant Y-movement in pixel/frame
	var santaMX = 1;			// steady movement's X-ratio
	var santaMY = 0;			// steady movement's Y-ratio
	var santaDX = 0;			// X-movement in pixel/frame, caused by storm
	var santaDY = 0;			// Y-movement in pixel/frame, caused by storm

	var flake    = new Array(flakes);
	var flakeX   = new Array(flakes);
	var flakeY   = new Array(flakes);
	var flakeSX  = new Array(flakes);
	var flakeVX  = new Array(flakes);
	var flakeVY	 = new Array(flakes);
	var flakeVIS = new Array(flakes);
	var flakeDX  = 0;			// X-movement in pixel/frame, caused by storm
	var flakeDY  = 0;			// Y-movement in pixel/frame, caused by storm



	var timer_id    = 0;		// ID if timer proc.
	var timer_sum   = refresh;	// Inital values for speed calculation
	var timer_count = 1;		// --''--

	var flake_visible = op;		// start with visble flakes for Opera, all others start invisible
	var flake_id	  = 0;		// timer id of make_flake_visible


	//-------------------------------------------------------------------------
	// preload images
	//-------------------------------------------------------------------------

	var kFlakeImages = 7;
	var flake_images = new Array(kFlakeImages);

	for(var i=0;i<kFlakeImages;i++)
	{
		flake_images[i] = new Image();
	}
	flake_images[0].src= snow0;
	flake_images[1].src= snow1;
	flake_images[2].src= snow2;
	flake_images[3].src= snow3;
	flake_images[4].src= snow4;
	flake_images[5].src= snow5;
	flake_images[6].src= snow6;

	var santa_image = new Image();
	santa_image.src = sleigh;

	//-------------------------------------------------------------------------
	// calculates optimum framerate & corresponding speed
	//-------------------------------------------------------------------------

	function rebuild_speed_and_timer()
	{
		var old = refresh_FperS;
		refresh = Math.floor(timer_sum/timer_count*2)+10;	// ms/Frame + spare
		refresh_FperS = Math.floor(1000/refresh);			// frames/second

		santa_speed = santa_speed_PperS/refresh_FperS;		// pixel/second  --> pixel/frame
		flake_speed = flake_speed_PperS/refresh_FperS;		// pixel/second  --> pixel/frame
		storm_speed = storm_speed_PperS/refresh_FperS; 		// pixel/second  --> pixel/frame

		if (timer_id) window.clearInterval(timer_id);		// adapt timer
		timer_id = window.setInterval(move_snow_and_santa,refresh);

		// FRAMES PER SECOND DISPLAY: REMOVE IF NOT NEEDED
		if(infoLayer){
			if (old!=refresh_FperS) write_to_layer(infoLayer,refresh_FperS+'f/s');
		}

		// adapt speed - for smoothness
		if (old != refresh_FperS) {
			var ratio = old/refresh_FperS;
			santaVY *= ratio;
			for (i=0; i<flakes; i++) {
				flakeSX[i] *= ratio;
				flakeVX[i] *= ratio;
				flakeVY[i] *= ratio;
			}
		}

		timer_count /= 2;	// moving medium
		timer_sum   /= 2;
	}

	//-------------------------------------------------------------------------
	// make flakes visible: while initalialisation phase flakes are invisble.
	//						after make_flakes_visible, all new flakes start visible
	//-------------------------------------------------------------------------

	function make_flake_visible_proc()
	{
		window.clearInterval(flake_id);
		flake_visible = true;
	}

	//-------------------------------------------------------------------------
	// storm proc - turn storm on/off
	//-------------------------------------------------------------------------

	function storm_proc()
	{
		// keep ourself from restarting while running
		window.clearInterval(storm_id);

		if (storm_blowing == 0) {
			// turn storm on
			storm_blowing = (Math.random()<0.5) ? -1 : 1 ;
			storm_YperX_current = Math.random()*storm_YperX;

			// prepare values by trigonometrical functions - too heavy for move_snow()
			var storm_theta = Math.atan(storm_YperX_current);
			storm_v_cos = Math.cos(storm_theta);
			storm_v_sin = Math.sin(storm_theta);
			storm_id = window.setInterval(storm_proc,storm_duration_S*1000.0);

		} else {
			// turn storm off
			storm_blowing *= 0.7;
			if ((Math.abs(storm_blowing)<0.05) || (!flake_visible)) {
				storm_blowing = 0;
				storm_id = window.setInterval(storm_proc,storm_lag_S*1000.0);
			} else {
				storm_id = window.setInterval(storm_proc,500.0);
			}
		}

		// preapare movement vektor for snow, caused by storm
		flakeDX = storm_v_cos*storm_speed*storm_blowing;
		flakeDY = Math.abs(storm_v_sin*storm_speed*storm_blowing);

		// preapare movement vektor for santa, caused by storm & santa_mass
		if(santa_mass>0) {
			santaDX = flakeDX/santa_mass;
			santaDY = flakeDY/santa_mass;
		} else {
			santaDX=0;
			santaDY=0;
		}
	}

	//-------------------------------------------------------------------------
	// create all layers & Objects
	//-------------------------------------------------------------------------

	function init_snow_and_santa()
	{
		// create santa
		santa   = get_layer_by_name('santa0');
		santaX  = santa_init_x;
		santaY  = Math.random()*pageHeight;
		santaSY = 0;

		if (santa_direction != 0) {
			santaMX =  Math.cos(santa_direction/180*Math.PI);
			santaMY = -Math.sin(santa_direction/180*Math.PI);
		}

		// create flake
		for (var i=0; i<flakes; i++) {
			flake[i]    = get_layer_by_name('flake'+i);
			flakeX[i]   = Math.random()*pageWidth;
			flakeY[i]   = Math.random()*pageHeight;
			flakeSX[i]  = 0;
			flakeVX[i]  = 0;
			flakeVIS[i] = flake_visible;
			flakeVY[i]  = 1;
		}
	}

	//-------------------------------------------------------------------------
	// get the named layer
	//-------------------------------------------------------------------------

	function get_layer_by_name(id)
	{
		if (op) 	{	return document.getElementById(id);	}
		if (ns6)	{	return document.getElementById(id);	}
		if (ie4)	{	return document.all[id];			}
		if (ns4)	{	return document.layers[id];			}
	}

	//-------------------------------------------------------------------------
	// move all objects
	//-------------------------------------------------------------------------

	function move_snow_and_santa()
	{
		var beginn = new Date().getMilliseconds();
		move_santa();
		move_snow();
		var ende = new Date().getMilliseconds();
		var diff = (beginn>ende?1000+ende-beginn:ende-beginn);
		timer_sum   += diff;
		timer_count ++;
		if (timer_count>10) {
			rebuild_speed_and_timer();
		}
	}

	//-------------------------------------------------------------------------
	// santa's private movement
	//-------------------------------------------------------------------------

	function move_santa()
	{
		var lmgn = -pageWidth*(1-santa_appearance)/santa_appearance;
		var rmgn = pageWidth;
		var h    = pageHeight+santa_image.height;

		// santa outside screen ?
		if (santaX > rmgn) {
			santaX  = lmgn;
			santaY  = Math.random()*pageHeight;
			santaSY = 0;
			santaVY = 0;
		} else if (santaX < lmgn) {
			santaX  = rmgn;
			santaY  = Math.random()*pageHeight;
			santaSY = 0;
			santaVY = 0;
		} else if (santaY >= pageHeight) {
			santaY -= h;
		} else if (santaY < -santa_image.height) {
			santaY += h;
		} else {
			santaX += santaMX * santa_speed + santaDX;
			santaY += santaMY * santa_speed + santaDY;
		}

		// up-down-movement
		santaSY --;
		if (santaSY <= 0) {
			santaSY = Math.random()*refresh_FperS*santa_TY;
			santaVY = (2.0*Math.random()-1.0)*santa_YperX*santa_speed;
		}

		// move santa to new position
		move_to(santa,santaX,santaY,(santaY<pageHeight- disappear_margin));
	}

	//-------------------------------------------------------------------------
	// snowflake's private movement
	//-------------------------------------------------------------------------

	function move_snow()
	{
		for (var i=0; i<flakes; i++) {
			// flake outside screen ?
			flakeX[i] += flakeVX[i] + flakeDX;
			flakeY[i] += flakeVY[i] + flakeDY;
			if (flakeY[i]>pageHeight-disappear_margin) {
				flakeX[i]  = Math.random()*pageWidth;
				flakeY[i]  = 0;
				flakeVY[i] = flake_speed+Math.random()*flake_speed;
				if (Math.random()<0.1) flakeVY[i] *= 2.0;
				if (flake_visible) flakeVIS[i] = true;
			}

			// left-right- movement
			flakeSX[i] --;
			if (flakeSX[i] <= 0) {
				flakeSX[i] = Math.random()*refresh_FperS*flake_TX;
				flakeVX[i] = (2.0*Math.random()-1.0)*flake_XperY*flake_speed;
			}

			if (flakeX[i]<-disappear_margin)
				flakeX[i] += pageWidth;
			if (flakeX[i]>=(pageWidth-disappear_margin))
				flakeX[i] -= pageWidth;

			// move flake to new position
			move_to(flake[i],flakeX[i],flakeY[i],flakeVIS[i]);
		}
	}

	//-------------------------------------------------------------------------
	// move a layer
	//-------------------------------------------------------------------------

	function move_to(obj, x, y, visible)
	{
		if (visible) {
			if (op) {
				obj.style.left		= x+"px";
				obj.style.top 		= y+"px";
				// FIX THIS: SHOW DOES NOT WORK OPERA
				obj.style.display 	= "block";
			} else if (ie4) {
				obj.style.pixelLeft = x;
				obj.style.pixelTop  = y;
				obj.style.visibility= "visible";
			} else if (ns4) {
				obj.left 			= x;
				obj.top				= y;
				obj.visibility 		= "show";
			} else if (ns6) {
				obj.style.left 		= x+"px";
				obj.style.top		= y+"px";
				obj.style.display 	= "block";
			}
		} else {
			if (ie4 || op) { obj.style.visibility = "hidden";}
			if (ns4) { obj.visibility 		= "hide";}
			if (ns6) { obj.style.display 	= "none";}
		}
	}

	//-------------------------------------------------------------------------
	// fill a layer with new content
	// --- parameter: layer-name, new content
	//-------------------------------------------------------------------------

	function write_to_layer(layer,txt)
	{
		if (op) {
			// FIX THIS: DOES NOT WORK FOR OPERA
			document.getElementById(layer).innerHTML = txt;
		} else if (ie4) {
			document.all[layer].innerHTML = txt;
		} else if (ns4) {
			document[layer].document.write(txt);
			document[layer].document.close();
		} else if (ns6) {
			var over = document.getElementById(layer);
			var range = document.createRange();
			range.setStartBefore(over);
			var domfrag = range.createContextualFragment(txt);
			while (over.hasChildNodes()) {
				over.removeChild(over.lastChild);
			}
			over.appendChild(domfrag);
		}
	}

	//-------------------------------------------------------------------------
	// size of page
	//-------------------------------------------------------------------------

	function get_page_dimension()
	{
		if(op) {
			pageOffX   = 0;
			pageOffY   = 25;
			pageWidth  = innerWidth  + pageOffX;
			pageHeight = innerHeight + pageOffY;
		} else if(ns6) {
			pageOffX   = scrollX;
			pageOffY   = scrollY;
			pageWidth  = innerWidth  + pageOffX;
			pageHeight = innerHeight + pageOffY;
		} else if(ns4) {
			pageOffX   = window.pageXOffset;
			pageOffY   = window.pageYOffset;
			pageWidth  = innerWidth  + pageOffX;
			pageHeight = innerHeight + pageOffY;
		} else if(ie4) {
			pageOffX   = document.body.scrollLeft;
			pageOffY   = document.body.scrollTop;
			pageWidth  = document.body.clientWidth  + pageOffX;
			pageHeight = document.body.clientHeight + pageOffY;
		}
	}

	//-------------------------------------------------------------------------
	// initialize all objects & timer
	//-------------------------------------------------------------------------

	export function start()
	{
		var a = '';

		// santa's private layer
		a += '<div id="santa0" '
		+  'style="position: absolute; '
		+  'left:-1px; top:-1px; z-index:0;">'
		+  '<img src="'+santa_image.src+'"></div>\n';

		// each snowflake's private layer
		for (var i=0; i<flakes; i++) {
			a += '<div id="flake'+i+'" '
			+  'style="position: absolute; '
			+  'left:-1px; top:-1px; z-index:0;">'
			+  '<img src="' +flake_images[i % kFlakeImages].src+ '"></div>\n';
		}


		document.body.innerHTML += a;

		// recalculate page dimension every second
		window.setInterval(get_page_dimension,1000);
		get_page_dimension();

		// init all objects
		init_snow_and_santa();

		// place snowflakes, santa & trees
		rebuild_speed_and_timer(refresh);
console.log(refresh)
		// start the animation
		timer_id = window.setInterval(move_snow_and_santa,refresh);
		storm_id = window.setInterval(storm_proc,1800);					// init with visible storm
		flake_id = window.setInterval(make_flake_visible_proc,2000);	// after the storm, let snowflakes fall :-)
	}


