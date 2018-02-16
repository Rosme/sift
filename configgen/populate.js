// MIT License
//
// Copyright (c) 2018 Jean-Sebastien Fauteux, Michel Rioux, Raphaël Massabot
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


function Rule(name)
{
  this.name = name;
  this.validScopes = [];
  this.parameter = "";
  this.enabled = false;
}

var rulesDef = [];
var ruleNames = [
  "NoAuto",
  "NoDefine",
  "NoMacroFunctions",
  "StartWithX",
  "EndWithX"
];

for(i in ruleNames)
{
  rulesDef.push(new Rule(ruleNames[i]));
}

var scopes = 
[
  "All",
  "Source",
  "Namespace",
  "Class",
  "Enum",
  "FreeFunction",
  "ClassFunction",
  "Function",
  "Conditionnal",
  "ClassVariable",
  "FunctionVariable",
  "GlobalVariable",
  "GlobalDefine",
  "Global",
  "Variable",
  "SingleLineComment",
  "MultiLineComment",
  "Comment"
];

var main = $("#main");
var rules = $("#rules");

function update_value_cb(cb){
  console.log(cb.currentTarget.id);
  for(i in rulesDef)
  {
    var current = rulesDef[i];
    if(current.name == cb.currentTarget.id)
    {
      console.log(cb.currentTarget.checked)
      rulesDef[i].enabled = cb.currentTarget.checked == true;
      break;
    }
  }
}

function update_value_list(e){
  console.log(e.currentTarget.id);
  id = e.currentTarget.id;
  var name = e.currentTarget.id.substring(3, id.length)
  for(i in rulesDef)
  {
    var current = rulesDef[i];
    if(current.name == name)
    {
      rulesDef[i].appliedTo = scopes[e.currentTarget.value];
      break;
    }
  }
}


function update_value_parameter(e){
  console.log(e.currentTarget.id);
  id = e.currentTarget.id;
  var name = e.currentTarget.id.substring(3, id.length)
  for(i in rulesDef)
  {
    var current = rulesDef[i];
    if(current.name == name)
    {
      rulesDef[i].parameter = e.currentTarget.value;
      break;
    }
  }
}

// Create rule buttons
for(i in rulesDef)
{
  var name = rulesDef[i].name;
  ruleDiv = document.createElement("div");
  var check = $('<input />', { type: 'checkbox', id: name, value: rulesDef[i].name });
  check.change(update_value_cb);
  $('<label />', { 'for': rulesDef[i].name, text: name }).appendTo(ruleDiv);
  check.appendTo(ruleDiv);
  
  var sel = $('<select />', { id: "sel"+name });
  for(var scope in scopes)
  {
    $('<option />', {value: scope, text: scopes[scope]}).appendTo(sel);
  }
  sel.change(update_value_list);
  sel.appendTo(ruleDiv);
  
  var param = $('<input />', { id: "tex"+name });
  param.on('input', update_value_parameter);
  param.appendTo(ruleDiv);
  rules.append(ruleDiv);
}


function createJsonString()
{
  var rules = {"rules":[]}
  for(i in rulesDef)
  {
    var rule = rulesDef[i];
    if(rule.enabled)
    {
      rules["rules"].push(
      {
        "rule" : rule.name,
        "appliedTo" : rule.appliedTo,
        "parameter" : rule.parameter
      });
    }
  }
  return JSON.stringify(rules, null, 2);
}

function exportAsJson(filename, elId, mimeType) {
  var elHtml = createJsonString();
  var link = document.createElement('a');
  mimeType = mimeType || 'text/plain';
  
  link.setAttribute('download', filename);
  link.setAttribute('href', 'data:' + mimeType  +  ';charset=utf-8,' + encodeURIComponent(elHtml));
  link.click(); 
}

$('#export').click(function(){
  exportAsJson("rules.json", 'main','text/json');
});
