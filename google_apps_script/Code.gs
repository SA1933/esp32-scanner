function doPost(e) {
  try {
    console.log("=== START POST REQUEST ===");
    console.log("Received POST request at: " + new Date());

    var ss = SpreadsheetApp.openById("your spreadsheet id here");
    var sheetStock = ss.getSheetByName("Stock");
    var sheetHistory = ss.getSheetByName("History");

    if (!sheetHistory) {
      sheetHistory = ss.insertSheet("History");
      sheetHistory.appendRow([
        "No", "date", "sku", "nama_barang", "type", "ip", "mutasi", "stock_bfr", "stock_aftr"
      ]);
    }

    var data = sheetStock.getDataRange().getValues();
    var headers = data[0];

    console.log("Headers found: " + headers.join(", "));

    var sku, jumlah;
    var parsedIp, parsedOps, parsedJenisIp;

    if (e.postData && e.postData.contents) {
      var contentType = e.postData.type || 'application/json';

      if (contentType.indexOf('application/json') !== -1) {
        var jsonData = JSON.parse(e.postData.contents);
        sku = jsonData.sku;
        jumlah = Number(jsonData.jumlah);
        parsedIp = String(jsonData.ip || '').trim();
        parsedOps = String(jsonData.ops || '').trim();
        parsedJenisIp = String(jsonData.jenis_ip || '').trim();
        console.log("Parsed from JSON - SKU: " + sku + ", Jumlah: " + jumlah + ", ip: " + parsedIp + ", jenis_ip: " + parsedJenisIp);
      } else if (contentType.indexOf('application/x-www-form-urlencoded') !== -1) {
        var params = e.postData.contents.split('&');
        for (var i = 0; i < params.length; i++) {
          var pair = params[i].split('=');
          if (pair[0] == 'sku') sku = decodeURIComponent(pair[1]);
          if (pair[0] == 'jumlah') jumlah = Number(decodeURIComponent(pair[1]));
          if (pair[0] == 'ip') parsedIp = String(decodeURIComponent(pair[1] || '')).trim();
          if (pair[0] == 'ops') parsedOps = String(decodeURIComponent(pair[1] || '')).trim();
          if (pair[0] == 'jenis_ip') parsedJenisIp = String(decodeURIComponent(pair[1] || '')).trim();
        }
        console.log("Parsed from Form - SKU: " + sku + ", Jumlah: " + jumlah + ", ip: " + parsedIp + ", jenis_ip: " + parsedJenisIp);
      }
    } else {
      sku = e.parameters.sku;
      jumlah = Number(e.parameters.jumlah);
      parsedIp = String(e.parameters.ip || '').trim();
      parsedOps = String(e.parameters.ops || '').trim();
      parsedJenisIp = String(e.parameters.jenis_ip || '').trim();
      console.log("Parsed from Parameters - SKU: " + sku + ", Jumlah: " + jumlah + ", ip: " + parsedIp + ", jenis_ip: " + parsedJenisIp);
    }

    if (!sku || isNaN(jumlah)) {
      console.log("ERROR: Parameter tidak valid - SKU: " + sku + ", Jumlah: " + jumlah);
      return ContentService.createTextOutput(JSON.stringify({
        status: "error",
        message: "Parameter tidak valid - SKU: " + sku + ", Jumlah: " + jumlah
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var skuColIndex = headers.indexOf("sku");
    var namaColIndex = headers.indexOf("nama_barang");
    var qtyColIndex = headers.indexOf("qty");

    console.log("Header indices - sku: " + skuColIndex + ", nama_barang: " + namaColIndex + ", qty: " + qtyColIndex);

    if (skuColIndex === -1) {
      console.log("ERROR: Kolom 'sku' tidak ditemukan di header");
      return ContentService.createTextOutput(JSON.stringify({
        status: "error",
        message: "Kolom 'sku' tidak ditemukan di spreadsheet"
      })).setMimeType(ContentService.MimeType.JSON);
    }

    if (qtyColIndex === -1) {
      console.log("ERROR: Kolom 'qty' tidak ditemukan di header");
      return ContentService.createTextOutput(JSON.stringify({
        status: "error",
        message: "Kolom 'qty' tidak ditemukan di spreadsheet"
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var found = false;
    var foundRow = -1;
    var currentQty = 0;
    var itemName = "";

    for (var i = 1; i < data.length; i++) {
      if (String(data[i][skuColIndex]).trim() === String(sku).trim()) {
        found = true;
        foundRow = i;
        currentQty = Number(data[i][qtyColIndex]) || 0;
        itemName = data[i][namaColIndex] || "Unknown";
        break;
      }
    }

    if (!found) {
      console.log("ERROR: SKU tidak ditemukan: " + sku);

      var existingSKUs = [];
      for (var j = 1; j < data.length; j++) {
        if (data[j][skuColIndex]) {
          existingSKUs.push(data[j][skuColIndex]);
        }
      }

      return ContentService.createTextOutput(JSON.stringify({
        status: "error",
        message: "SKU tidak ditemukan: " + sku,
        existing_skus: existingSKUs
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var lock = LockService.getScriptLock();
    lock.waitLock(10000);
    try {
      var newQty = currentQty + jumlah;
      sheetStock.getRange(foundRow + 1, qtyColIndex + 1).setValue(newQty);

      var timestamp = new Date();
      var ip = parsedIp || "unknown";
      var ops = (parsedOps === 'in' || parsedOps === 'out') ? parsedOps : (jumlah >= 0 ? 'in' : 'out');

      console.log('doPost debug - parsedIp=' + parsedIp + ', parsedOps=' + parsedOps + ', jenis_ip=' + parsedJenisIp + ', final ip=' + ip + ', final ops=' + ops);

      var formattedDate = Utilities.formatDate(timestamp, Session.getScriptTimeZone(), "dd/MM/yyyy");

      var lastHistoryRow = sheetHistory.getLastRow();
      var autoNumber = 1;

      if (lastHistoryRow > 1) {
        var lastNo = sheetHistory.getRange(lastHistoryRow, 1).getValue();
        autoNumber = (lastNo && !isNaN(lastNo)) ? Number(lastNo) + 1 : lastHistoryRow;
      }

      sheetHistory.appendRow([
        autoNumber,
        formattedDate,
        sku,
        itemName,
        ops,
        ip,             
        jumlah,              
        currentQty,        
        newQty          
      ]);

      var newHistoryRow = sheetHistory.getLastRow();
      sheetHistory.getRange(newHistoryRow, 2).setNumberFormat("dd/mm/yyyy");

      var ipCell = sheetHistory.getRange(newHistoryRow, 6);
      ipCell.setNumberFormat("@");
      ipCell.setValue(String(ip));
      
    } finally {
      try { lock.releaseLock(); } catch (er) { /* ignore */ }
    }

    console.log("SUCCESS: Updated SKU " + sku + " (" + itemName + ") from " + currentQty + " to " + newQty);

    return ContentService.createTextOutput(JSON.stringify({
      status: "success",
      message: "SKU " + sku + " diperbarui dari " + currentQty + " menjadi " + newQty,
      data: {
        sku: sku,
        nama_barang: itemName,
        previous_qty: currentQty,
        new_qty: newQty,
        operation: ops,
        timestamp: timestamp.toISOString(),
        ip: ip
      }
    })).setMimeType(ContentService.MimeType.JSON);
    
  } catch (err) {
    console.error("ERROR: " + err.message);
    console.error("Stack: " + err.stack);

    return ContentService.createTextOutput(JSON.stringify({
      status: "error",
      message: "Server error: " + err.message
    })).setMimeType(ContentService.MimeType.JSON);
  }
}

// Fungsi doGet tetap sama seperti sebelumnya
function doGet(e) {
  try {
    var sku = e.parameter.sku;
    var jumlah = e.parameter.jumlah;

    if (e.parameter.info == '1' && sku) {
      var ss = SpreadsheetApp.openById("//your spreadsheet id here//");
      var sheetStock = ss.getSheetByName("Stock");
      var sheetDataSKU = ss.getSheetByName("Data SKU");
      var stockData = sheetStock.getDataRange().getValues();
      var stockHeaders = stockData[0];
      var skuIdx = stockHeaders.indexOf('sku');
      var qtyIdx = stockHeaders.indexOf('qty');
      var nameShort2 = "";
      var nameShort3 = "";
      var stockNow = 0;

      if (sheetDataSKU) {
        var ds = sheetDataSKU.getDataRange().getValues();
        // Asumsi: A=No, B=sku, C=nama_barang, D=nama_barang_baris-2, E=nama_barang_baris-3
        for (var i = 1; i < ds.length; i++) {
          if (String(ds[i][1]).trim() === String(sku).trim()) {
            nameShort2 = String(ds[i][3] || ds[i][2] || '').trim();
            nameShort3 = String(ds[i][4] || '').trim();
            break;
          }
        }
      }

      for (var j = 1; j < stockData.length; j++) {
        if (String(stockData[j][skuIdx]).trim() === String(sku).trim()) {
          stockNow = Number(stockData[j][qtyIdx]) || 0;
          break;
        }
      }

      return ContentService.createTextOutput(JSON.stringify({
        ok: true,
        short2: nameShort2,
        short3: nameShort3,
        stock: stockNow
      })).setMimeType(ContentService.MimeType.JSON);
    }

    if (sku && jumlah) {
      var mockPost = {
        parameters: {
          sku: sku,
          jumlah: jumlah,
          ip: e.parameter.ip || '',
          ops: e.parameter.ops || ''
        }
      };
      return doPost(mockPost);
    }

    var ss = SpreadsheetApp.openById("your spreadsheet id here");
    var sheet = ss.getSheets()[0];
    var data = sheet.getDataRange().getValues();
    var headers = data[0];

    var skuList = [];
    for (var i = 1; i < Math.min(data.length, 11); i++) {
      if (data[i][headers.indexOf("sku")]) {
        skuList.push({
          sku: data[i][headers.indexOf("sku")],
          nama_barang: data[i][headers.indexOf("nama_barang")],
          qty: data[i][headers.indexOf("qty")]
        });
      }
    }

    return ContentService.createTextOutput(JSON.stringify({
      status: "info",
      message: "Google Apps Script Ready - Connected to Spreadsheet",
      spreadsheet_info: {
        total_rows: data.length,
        headers: headers,
        sample_data: skuList
      },
      usage: {
        post_method: "Send POST request with JSON:{\"sku\":\"SKU\",\"jumlah\":1}",
        get_info: "GET ?sku=SKU&info=1 returns {short,stock}",
        get_test: "GET ?sku=SKU&jumlah=1 to simulate update"
      }
    })).setMimeType(ContentService.MimeType.JSON);

  } catch (err) {
    return ContentService.createTextOutput(JSON.stringify({
      status: "error",
      message: "Error: " + err.message
    })).setMimeType(ContentService.MimeType.JSON);
  }
}

function setupHistorySheet() {
  var ss = SpreadsheetApp.openById("your spreadsheet id here");
  var sheetHistory = ss.getSheetByName("History");
  
  if (!sheetHistory) {
    sheetHistory = ss.insertSheet("History");
    sheetHistory.appendRow([
      "No", "date", "sku", "nama_barang", "type", "ip", "mutasi", "stock_bfr", "stock_aftr"
    ]);

    var headerRange = sheetHistory.getRange(1, 1, 1, 9);
    headerRange.setFontWeight("bold");
    headerRange.setBackground("#4a86e8");
    headerRange.setFontColor("white");

    sheetHistory.setColumnWidth(1, 50);
    sheetHistory.setColumnWidth(2, 100); 
    sheetHistory.setColumnWidth(3, 100);  
    sheetHistory.setColumnWidth(4, 300);
    sheetHistory.setColumnWidth(5, 80); 
    sheetHistory.setColumnWidth(6, 120);
    sheetHistory.setColumnWidth(7, 80); 
    sheetHistory.setColumnWidth(8, 80); 
    sheetHistory.setColumnWidth(9, 80); 

    sheetHistory.setFrozenRows(1);
    
    return "History sheet created with proper structure!";
  } else {
    return "History sheet already exists with " + (sheetHistory.getLastRow() - 1) + " records.";
  }
}
