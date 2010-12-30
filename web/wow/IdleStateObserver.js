dojo.provide("wow.IdleStateObserver");

dojo.declare("wow.IdleStateObserver", null, {
  constructor: function(grid, labels){
    this.grid = grid;
    this.idle = false;
    this.labels = labels;
    this._displayLeft = true;
    this._gridNodes = dojo.NodeList();
    this._generation = 0;
  },
  
  observe: function(activeTopic, idleTopic){
    dojo.subscribe(activeTopic, this, "setActive");
    dojo.subscribe(idleTopic, this, "setIdle");
  },
  
  setIdle: function(){
    this._idle = true;
    this._hideAnim && this._hideAnim.stop(true);
    
    this._gridNodes.push(
      this.grid.create("center", "p", {
        innerHTML: this.labels.center,
        style: {
          opacity: 0,
          height: 0,
          paddingTop: "24px"
        }
      }),
      this.grid.create(this._displayLeft ? "left" : "right", "p", {
        innerHTML: this._displayLeft ? this.labels.left : this.labels.right,
        style: {
          opacity: 0,
          height: 0
        }
      })
    );
    this._displayLeft = !this._displayLeft;
    
    // Fade in now
    this._showAnim = dojo.fx.combine(this._gridNodes.reduce(function(anims, node){
      anims.push(dojo.fx.chain([
        dojo.fx.wipeIn({ node: node }),
        dojo.fadeIn({ node: node, start: 0, duration: 1000 })
      ]));
      return anims;
    }, [])).play();
    
    // Fade out after delay
    this._hideAnim = dojo.fx.combine(this._gridNodes.reduce(function(anims, node){
      anims.push(
        dojo.fadeOut({
          node: node,
          start: 1,
          end: 0,
          duration: 1000
        })
      );
      return anims;
    }, []));
    dojo.connect(this._hideAnim, "onEnd", this, function(){
      this._hideAnim = null;
      this._gridNodes.filter(function(node){ dojo.destroy(node); });
      this._idle && this.setIdle();
    });
    var timeoutGeneration = ++this._generation;
    setTimeout(dojo.hitch(this, function(){
      if(timeoutGeneration === this._generation && this._idle && this._hideAnim){
        this._hideAnim.play();
      }
    }), 8000);
  },
  
  setActive: function(){
    this._idle = false;
    this._showAnim && this._showAnim.stop();
    this._hideAnim && this._hideAnim.play();
  }
});
