dojo.provide("wow.TagObserver");

dojo.declare("wow.TagObserver", null, {
  constructor: function(grid, anonymousLabels, labels){
    this.grid = grid;
    this.anonymousLabels = anonymousLabels;
    this.labels = labels;
  },
  
  observe: function(tagTopic){
    dojo.subscribe(tagTopic, this, "displayMessage");
  },
  
  displayMessage: function(msg){
    var node = this.grid.create(msg.event, "p", {
      innerHTML: msg.user ? msg.user.title + " " + this.labels[msg.event] : this.anonymousLabels[msg.event],
      style: {
        opacity: 0,
        height: 0
      }
    });
    
    dojo.fx.chain([
      dojo.fx.wipeIn({ node: node }),
      dojo.fadeIn({ node: node, start: 0, duration: 1000 })
    ]).play();
    
    dojo.fadeOut({
      node: node,
      start: 1,
      end: 0,
      duration: 1000,
      onEnd: function(){
        dojo.destroy(node);
      }
    }).play(8000);
  }
});
