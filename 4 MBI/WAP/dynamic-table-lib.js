/**
 * WAP Project - Dynamic table (element of page)
 * @Author: Filip Gulan
 * @Mail: xgulan00@stud.fit.vutbr.cz
 * @Date: 16.4.2018
 * @Name: dynamic-table-li.js
 */

/**
 * Get values of all filters
 * @param table
 * @returns {Array} - values of filters in array
 */
function getFiltersValues(table) {
  var filters = [];
  for(var i = 0; i < table.tHead.rows[0].cells.length; i++) { //iterate trought first row to get values of inputs
    filters.push(table.tHead.rows[0].cells[i].getElementsByTagName('input')[0].value);
  }
  return filters;
}

/**
 * Filtrate for every keyup event depends on filters values
 * @param table
 */
function filterColumns(table) {
  var tBody = table.tBodies[0];
  var filters = getFiltersValues(table);
  if(!tBody) { //there is no tBody for filtration
    return
  }
  for(var i = 0; i < tBody.rows.length; i++) { //iterate trought rows
    var suitsFilter = true;
    for(var j = 0; j < tBody.rows[i].cells.length; j++) { //iterate trought columns
      if(tBody.rows[i].cells[j].textContent.indexOf(filters[j]) === -1) {
        suitsFilter = false;
        break;
      }
    }
    if(suitsFilter === true) { //if suits for any filter then we display this row
      tBody.rows[i].style.display = '';
    } else { //if do not suits for any filter then we do not display row
      tBody.rows[i].style.display = 'none';
    }
  }
}

/**
 * Create input filter
 * @param table
 * @returns {HTMLInputElement} - created input filter
 */
function createFilter(table) {
  var input = document.createElement('input');
  input.type = 'text';
  input.onkeyup = function() {
    filterColumns(table);
  };
  return input;
}

/**
 * Apply order arrow style to div
 * @param arrow - div
 * @param color - of arrow
 * @param direction - up/down diection
 */
function applyOrderArrowStyle(arrow, color, direction) {
  var rotate = (direction === 'up') ? 'rotate(-135deg)' : 'rotate(45deg)';
  arrow.style.border = 'solid ' + color;
  arrow.style['border-width'] = '0 3px 3px 0';
  arrow.style.display = 'inline-block';
  arrow.style.padding = '3px';
  arrow.style.cursor = 'pointer';
  arrow.style.transform = rotate;
  arrow.style['-webkit-transform'] = rotate;
  arrow.setAttribute('data-direction', direction);
}

/**
 * Reset order arrow styles
 * @param tableName - id of table
 */
function resetOrder(tableName) {
  var table = document.getElementById(tableName);
  var orderArrowList = table.getElementsByClassName(tableName + '-arrow');
  for(var i = 0; i < orderArrowList.length; i++) { //iterate trought  arrows and set default style
    applyOrderArrowStyle(orderArrowList[i], '#c6c6c6', 'up');
  }
}

/**
 * Switch rows by order
 * @param i - first row index
 * @param j - second row index
 * @param index - of column
 * @param tBody - element
 * @param type - string or number sorting
 * @param direction - down/up sort direction
 */
function switchRows(i, j, index, tBody, type, direction) {
  var row1 = tBody.rows[i].getElementsByTagName("td")[index].textContent.toLowerCase();
  var row2 = tBody.rows[j].getElementsByTagName("td")[index].textContent.toLowerCase();
  if(type === 'number') { //if it is number
    row1 = parseFloat(row1);
    row2 = parseFloat(row2);
  }
  if(direction === 'down'){ //from low to high
    if(row1 > row2) {
      tBody.rows[i].parentNode.insertBefore(tBody.rows[j], tBody.rows[i]);
    }
  } else { //from high to low
    if(row1 < row2) {
      tBody.rows[i].parentNode.insertBefore(tBody.rows[j], tBody.rows[i]);
    }
  }
}

/**
 * Order items in tBody
 * @param tBody - element
 * @param index - index of column
 * @param type - string or number sorting
 * @param direction - down/up sort direction
 */
function order(tBody, index, type, direction) {
  if(!tBody) { //there is no tBody
    return;
  }
  if(tBody.rows.length > 1) { //tBody has got more than 1 rows to sort
    for(var i = 0; i < tBody.rows.length; i++) { //iterate thought rows
      for(var j = i + 1; j < tBody.rows.length; j++) { //iterate thought rows
        switchRows(i, j, index, tBody, type, direction);
      }
    }
  }
}

/**
 * Create arrow element
 * @param tableName
 * @param index
 * @returns {HTMLDivElement} - arrow div element
 */
function createOrder(tableName, index) {
  var table = document.getElementById(tableName);
  var tBody = table.tBodies[0];
  var arrow = document.createElement('div');
  arrow.classList.add(tableName + '-arrow');
  arrow.setAttribute('data-index', index);
  applyOrderArrowStyle(arrow, '#c6c6c6', 'up');
  arrow.onclick = function(element) {
    var direction = (element.target.getAttribute('data-direction') === 'up') ? 'down' : 'up';
    var type = (element.target.parentElement.getAttribute('data-type')) ? element.target.parentElement.getAttribute('data-type') : 'string';
    resetOrder(tableName);
    applyOrderArrowStyle(element.target, 'black', direction);
    order(tBody, element.target.getAttribute('data-index'), type, direction);
  };
  return arrow;
}

/**
 * Function to make table dynamics
 * @param tableName - id of table to make dynamics
 */
function makeTableDynamic(tableName) {
  var table = document.getElementById(tableName);
  if(!table) { //there is no table with given id
    return;
  }
  if(!table.tHead) { //thead do not exist then we create thead for filters
    var tableColumns = table.rows[0].cells.length;
    var headRow = table.createTHead().insertRow();
    for(var i = 0; i < tableColumns; i++) { //we create row and cell for thead
      headRow.appendChild(document.createElement('th'));
    }
  }
  for(var i = 0; i < table.rows[0].cells.length; i++) { //create filters and order arrows for first head rows columns
    table.tHead.rows[0].cells[i].appendChild(createOrder(tableName, i));
    table.tHead.rows[0].cells[i].appendChild(document.createElement('br'));
    table.tHead.rows[0].cells[i].appendChild(createFilter(table));
  }
}