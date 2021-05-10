// Get interface list
$(document).ready(function() {

});

var timer;
var current_query_para;

$("#updateInterface").click(function () {
    var agent_addr = document.forms["topkQueryPlan"]["agentAddress"].value;
    var htmlobj = $.ajax({ url: "/getiflist", data: {agentAddress: agent_addr}, async: false});
    var iflist = JSON.parse(htmlobj.responseText);
    $("#interfaceName").children().remove();
    for (var interface_info of iflist) {
        $("#interfaceName").append("<option>" + interface_info[0] + "</option>");
    }
});

// Post capture parameters
$("#topkStart").click(function() {
    var ifname = document.forms["topkQueryPlan"]["interfaceName"].value;
    var pktCount = document.forms["topkQueryPlan"]["pktCount"].value;
    var k = document.forms["topkQueryPlan"]["k"].value;
    current_query_para = {ifname: ifname, pktCount: pktCount, k: k};
    $.ajax({
        type: "GET", 
        url: "/run_capture", 
        data: current_query_para, 
        success: function(result) {
            $("#result").removeAttr("hidden");
            $("#capture-progress").attr("style", "width: 0%");
            $("#result-table tbody").children().remove();
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
            for (var entry of topk_list) {
                var flow_detail = JSON.parse(entry[1]);
                $("#result-table tbody").append('<tr> <th scope="row">' + entry[0] + '</th> <td>' + flow_detail.type + '</td> <td>' + flow_detail.src_ip + '</td> <td>' + flow_detail.src_port + '</td> <td>' + flow_detail.dst_ip + '</td> <td>' + flow_detail.dst_port + '</td> <td>' + entry[2] + '</td> </tr>');
            }
            $("#result-table").removeAttr("hidden");
        }
    });
}

