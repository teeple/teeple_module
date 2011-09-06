if ( Drupal.jsEnabled) {
	$(document).ready( function() {
      $(document).bind('flagGlobalAfterLinkUpdate', function(event, data) {
		//alert( 'checkin_count' + Drupal.settings.tvchat.checkin_count + data.contentId + '<br>');

		// get checkin count
	    $("#tvchat_checkin_count_" + data.contentId).load( 
		  Drupal.settings.tvchat.checkin_count + '?nid=' + data.contentId, 
		  function( response) {
		    $(this).text( $(response).text());
		  });
      });
    });
}
