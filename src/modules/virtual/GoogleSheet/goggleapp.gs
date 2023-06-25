
var SS = SpreadsheetApp.openById('GOOGLE_SHEET_ID');

function doGet(e){
  
	var value = e.parameters.value;
	var id = e.parameters.id;
	var sheet = e.parameter.sheet;
	var init = e.parameter.init;

	if (sheet == undefined) {
		sheet = SS.getActiveSheet();
	} else{
		sheet = SS.getSheetByName(sheet);
	}

	if (sheet == null) {
		return ContentService.createTextOutput("Error sheet name");
	}

//	if (init !== undefined){
		sheet.getRange('A1').setValue("ID"); 
		sheet.getRange('B1').setValue("VALUE");
		sheet.getRange('C1').setValue("DATE");
//	}
  
	var nextFreeRow = sheet.getLastRow() + 1;
  
	if (id == undefined) id = "nan";
	if  (value !== undefined){
		try {  
			var now = Utilities.formatDate(new Date(), "GMT+03:00", "dd.MM.yyyy HH:mm:ss");
			for (let i = 0; i < id.length; i++) {
				sheet.getRange("A"+nextFreeRow).setValue(id[i]);
				value[i] = value[i].replace(/\./, ",");
				sheet.getRange("B"+nextFreeRow).setValue(value[i]);
				sheet.getRange("C"+nextFreeRow).setValue(now);
				nextFreeRow++;
			}
			return ContentService.createTextOutput("Successfully wrote: " +
				e.parameter.value + "\ninto spreadsheet.");
			//Logger.log(str);
		} 
		catch(f){
			return ContentService.createTextOutput("Error in parsing appendRow");
		}
	} else {
		return ContentService.createTextOutput("Error wrote: value = " +
			e.parameter.value + " , id = "+e.parameter.id);
	}
}

