dojo.provide("wow.Grid");

dojo.declare("wow.Grid", null, {
  constructor: function(mapping){
    this.mapping = mapping;
  },
  
  create: function(key, tagName, attrs){
    return dojo.create(tagName, attrs, this.mapping[key], "first");
  }
});
