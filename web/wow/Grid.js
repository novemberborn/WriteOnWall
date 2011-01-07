dojo.provide("wow.Grid");

dojo.declare("wow.Grid", null, {
  constructor: function(mapping){
    this.mapping = mapping;
  },
  
  create: function(key, tagName, attrs){
    return dojo.create(tagName, attrs, this.mapping[key], "first");
  },
  
  overlayGaps: function(nodes, color){
    if(nodes.length != 6){
      console.error("Can't overlay gaps, 6 blocks expected, got %s", nodes.length);
      return;
    }
    
    var blocks = new wow.Grid._BlockList;
    var allCoords = nodes.map(function(node){ return dojo.coords(node); });
    allCoords.sort(function(a, b){
      if(a.y < b.y){
        return -1;
      }else if(a.y > b.y){
        return 1;
      }else if(a.h < b.h){
        return -1;
      }else if(a.h > b.h){
        return 1;
      }else if(a.x < b.x){
        return -1;
      }else{
        return 1;
      }
    });
    allCoords.forEach(function(coords){
      blocks.add(coords.x, coords.y, coords.w, coords.h);
    });
    
    var page = { w: document.documentElement.scrollWidth, h: document.documentElement.scrollHeight };
    this._overlays = dojo.NodeList();
    allCoords.forEach(dojo.hitch(this, "_addOverlays", blocks, page));
    this._overlays.style("backgroundColor", color);
    this._overlays.place(dojo.body());
  },
  
  _addOverlays: function(blocks, page, coords){
    var w = coords.w, h = coords.h;
    var t = coords.y, r = coords.x + w, b = coords.y + h, l = coords.x;
    
    // Overlay to the left
    var x1 = 0;
    var existing = blocks.find("left", x1, t, l, h);
    if(existing){
      x1 = Math.min(l, existing.r);
    }
    if(x1 < l){
      this._overlays.push(this._createOverlay(x1, t, l - x1, h));
      blocks.add(x1, t, l - x1, h);
    }
    
    // Overlay to the right
    var x2 = page.w;
    var existing = blocks.find("right", r, t, x2 - r, h);
    if(existing){
      x2 = existing.l;
    }
    if(x2 > r){
      this._overlays.push(this._createOverlay(r, t, x2 - r, h));
      blocks.add(r, t, x2 - r, h);
    }
    
    if(t > 0){
      w = x2 - x1;
      
      var existing = blocks.find("above", x1, 0, w, t);
      if(!existing.length){
        this._overlays.push(this._createOverlay(x1, 0, w, t));
        blocks.add(x1, 0, w, t);
      }else{
        var first = existing[0];
        if(first.l > x1){
          this._overlays.push(this._createOverlay(x1, 0, first.l - x1, t));
          blocks.add(x1, 0, first.l - x1, t);
        }
        dojo.forEach(existing, function(block){
          var blockL = Math.max(x1, block.l);
          var blockW = block.r - blockL;
          x1 += blockW;
          w -= blockW;
          if(block.b < t){
            this._overlays.push(this._createOverlay(blockL, block.b, blockW, t - block.b));
            blocks.add(blockL, block.b, blockW, t - block.b);
          }
        }, this);
        if(w > 0){
          this._overlays.push(this._createOverlay(x1, 0, w, t));
          blocks.add(x1, 0, w, t);
        }
      }
    }
    
    blocks.consolidate();
  },
  
  _createOverlay: function(x, y, w, h, className){
    return dojo.create("div", {
      style: {
        position: "absolute",
        left: x + "px",
        top: y + "px",
        width: w + "px",
        height: h + "px"
      }
    });
  }
});

wow.Grid._BlockList = dojo.extend(function(){ return dojo.mixin([], this.constructor.prototype); }, {
  add: function(l, t, w, h){
    this.push({ t: t, r: l + w, b: t + h, l: l });
  },
  
  willOverlap: function(l, t, w, h){
    var b = t + h, r = l + w;
    for(var i = 0, overlap, block; !overlap && (block = this[i]); i++){
      overlap = (block.r > l && block.r <= r || block.l < r && block.r >= r || block.l >= l && block.r <= r || block.l <= l && block.r >= r)
          && (block.b > t && block.b <= b || block.t < b && block.b >= b || block.t >= t && block.b <= b || block.t <= t && block.b >= b);
    }
    return overlap;
  },
  
  find: function(d, l, t, w, h){
    var existing = [];
    var b = t + h, r = l + w;
    for(var i = 0, overlap, block; block = this[i]; i++){
      overlap = (block.r > l && block.r <= r || block.l < r && block.r >= r || block.l >= l && block.r <= r || block.l <= l && block.r >= r)
          && (block.b > t && block.b <= b || block.t < b && block.b >= b || block.t >= t && block.b <= b || block.t <= t && block.b >= b);
      if(overlap){
        existing.push(block);
      }
    }

    switch(d){
      case "left":
        return existing.sort(function(a, b){ return a.r > b.r ? -1 : a.r < b.r ? 1 : a.b > b.b ? 1 : -1; })[0];
      case "right":
        return existing.sort(function(a, b){ return a.l < b.l ? -1 : a.l > b.l ? 1 : a.b > b.b ? 1 : -1; })[0];
      case "above":
        existing.sort(function(a, b){ return a.b > b.b ? -1 : a.b < b.b ? 1 : a.l < b.l ? -1 : 1; });
        var result = [];
        var rightmost = 0, leftmost = r;
        for(var i = 0, block; rightmost < r && (block = existing[i]);){
          if(block.r <= rightmost){
            i++;
            continue;
          }

          result.push({
            t: block.t,
            r: Math.min(r, block.r),
            b: block.b,
            l: block.l
          });

          rightmost = Math.min(r, block.r);
          leftmost = Math.min(leftmost, block.l);
          existing.splice(i, 1);
        }
        for(var i = 0, block; leftmost > l && (block = existing[i]); i++){
          if(block.l > leftmost){
            continue;
          }

          result.unshift({
            t: block.t,
            r: leftmost,
            b: block.b,
            l: block.l
          });

          leftmost = block.l;
        }
        return result;
    }
  },
  
  consolidate: function(start){
    this.sort(function(a, b){ return a.t < b.t ? -1 : a.t > b.t ? 1 : a.l < b.l ? -1 : 1; });
    for(var i = start || 0, block, next; (block = this[i]) && (next = this[i + 1]); i++){
      if(block.t == next.t && block.r == next.l){
        this.splice(i, 2);
        var h = Math.min(block.b - block.t, next.b - next.t);
        this.add(block.l, block.t, next.r - block.l, h);
        if(block.b - block.t > h){
          this.add(block.l, block.t + h, block.r - block.l, block.b - block.t - h);
        }
        if(next.b - next.t > h){
          this.add(next.l, next.t + h, next.r - next.l, next.b - next.t - h);
        }
        return this.consolidate(i);
      }
    }
  }
});
