/**
 * Google Apps Script for ESP32 Barcode Scanner
 * 
 * This script receives barcode data from ESP32 devices and updates a Google Sheet
 * 
 * Setup Instructions:
 * 1. Create a new Google Sheet with two tabs: "Stock" and "History"
 * 2. In "Stock" tab, add headers: Barcode, Item Name, Quantity, Last Updated
 * 3. In "History" tab, add headers: Timestamp, Device ID, Barcode, Quantity, Action
 * 4. Open Tools > Script Editor and paste this code
 * 5. Deploy as Web App (Deploy > New deployment > Web app)
 * 6. Set "Execute as" to your account
 * 7. Set "Who has access" to "Anyone" (for HTTP POST from ESP32)
 * 8. Copy the web app URL and configure it in the ESP32
 */

// Main function to handle POST requests from ESP32
function doPost(e) {
  try {
    // Parse JSON payload
    var data = JSON.parse(e.postData.contents);
    
    var deviceId = data.deviceId;
    var barcode = data.barcode;
    var quantity = parseInt(data.quantity);
    var timestamp = data.timestamp;
    
    Logger.log("Received: " + JSON.stringify(data));
    
    // Get spreadsheet
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var stockSheet = ss.getSheetByName("Stock");
    var historySheet = ss.getSheetByName("History");
    
    // Update stock
    updateStock(stockSheet, barcode, quantity);
    
    // Log history
    logHistory(historySheet, deviceId, barcode, quantity, timestamp);
    
    // Return success response
    return ContentService.createTextOutput(JSON.stringify({
      'status': 'success',
      'message': 'Data received and processed',
      'barcode': barcode,
      'newQuantity': getStockQuantity(stockSheet, barcode)
    })).setMimeType(ContentService.MimeType.JSON);
    
  } catch (error) {
    Logger.log("Error: " + error.toString());
    
    return ContentService.createTextOutput(JSON.stringify({
      'status': 'error',
      'message': error.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  }
}

// Update stock quantity in Stock sheet
function updateStock(sheet, barcode, quantity) {
  var data = sheet.getDataRange().getValues();
  var found = false;
  
  // Search for existing barcode
  for (var i = 1; i < data.length; i++) {
    if (data[i][0] == barcode) {
      // Update existing row
      var currentQty = parseInt(data[i][2], 10) || 0;
      var newQty = currentQty + quantity;
      
      sheet.getRange(i + 1, 3).setValue(newQty); // Update quantity
      sheet.getRange(i + 1, 4).setValue(new Date()); // Update timestamp
      
      found = true;
      break;
    }
  }
  
  // If not found, add new row
  if (!found) {
    sheet.appendRow([
      barcode,
      "Item-" + barcode, // Placeholder name
      quantity,
      new Date()
    ]);
  }
}

// Log transaction in History sheet
function logHistory(sheet, deviceId, barcode, quantity, deviceTimestamp) {
  var action = quantity > 0 ? "ADD" : "REMOVE";
  
  sheet.appendRow([
    new Date(),
    deviceId,
    barcode,
    quantity,
    action,
    deviceTimestamp
  ]);
}

// Get current stock quantity for a barcode
function getStockQuantity(sheet, barcode) {
  var data = sheet.getDataRange().getValues();
  
  for (var i = 1; i < data.length; i++) {
    if (data[i][0] == barcode) {
      return parseInt(data[i][2], 10) || 0;
    }
  }
  
  return 0;
}

// Optional: Handle GET requests for testing
function doGet(e) {
  return ContentService.createTextOutput(JSON.stringify({
    'status': 'ok',
    'message': 'ESP32 Scanner API is running',
    'version': '1.0'
  })).setMimeType(ContentService.MimeType.JSON);
}
