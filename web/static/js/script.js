// Get interface list
$(document).ready(function() {
    var htmlobj = $.ajax({url: "/getiflist", async: false});
    var iflist = JSON.parse(htmlobj.responseText);
    for (var interface_info of iflist) {
        $("#interfaceName").append("<option>" + interface_info[0] + "</option>");
    }
});

var timer;
var current_query_para;

// Post capture parameters
$("#topkStart").click(function() {
    ifname = document.forms["topkQueryPlan"]["interfaceName"].value;
    pktCount = document.forms["topkQueryPlan"]["pktCount"].value;
    k = document.forms["topkQueryPlan"]["k"].value;
    current_query_para = {ifname: ifname, pktCount: pktCount, k: k};
    $.ajax({
        type: "GET", 
        url: "/run_capture", 
        data: current_query_para, 
        success: function(result) {
            $("#result").removeAttr("hidden");
            $("#capture-progress").attr("style", "width: 0%");
            $("#result-table tbody").remove();
            $("#result-table").append("<tbody> </tbody>");
            timer = setInterval(check_progress, 1000)
            console.log(result);
        }
    });
    return false;
});

function check_progress() {
    $.ajax({
        type: "GET", 
        url: "/get_progress", 
        success: function(result) {
            if (result == "Done") {
                $("#capture-progress").attr("style", "width: 100%");
                clearInterval(timer);
                get_result();
            } else {
                $("#capture-progress").attr("style", "width: " + (result / current_query_para.pktCount) * 100 + "%");
            }
        }
    });
};

function get_result() {
    $.ajax({
        type: "GET", 
        url: "/get_result", 
        success: function(result) {
            var topk_list = JSON.parse(result);
            console.log(topk_list);
            var i = 1;
            for (var entry of topk_list) {
                $("#result-table tbody").append('<tr> <th scope="row">' + i + '</th> <td>' + entry[0] + '</td> <td>' + entry[1] + '</td> </tr>');
                i++
            }
            $("#result-table").removeAttr("hidden");
        }
    });
}

