using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class TaskMan3 : MonoBehaviour
{
    
    //Array of strings holding the text for each task
    public string[] taskText;

    //Text object
    public TMPro.TextMeshProUGUI directions;

    //Objects relating to the pipes 
    public Pipeline[] pipes;
    public GameObject[] pipeObjs;
    public GameObject[] whiteMats;

    //UI Elements
    public GameObject nextButton;
    public GameObject quizPanel;
    public GameObject ansPanel;

    //Materials for highlighting the transparent pipes
    public Material hiMat;
    public Material defMat;
    public bool isNextRunning = false;

    //Running tally for the current task
    private int counter = 0;

    //Used to hold each segment of an protein
    private Transform obj1;
    private Transform obj2;
    private Transform obj3;
    private Transform obj4;
    

    //Initialize the text
    void Start()
    {
        directions.text = taskText[0];
    }

    
    void Update()
    {
        Debug.Log(counter);
        var exitState = new int[5];
        switch (counter)
        {
            case 0:
                break;

            //3 Na+ goes through the Na K+ ATPase Protein to the extracellular space
            case 1:
                whiteMats[0].SetActive(true);
                pipeObjs[1].SetActive(false);
                exitState = pipes[0].exitState;

                //Get Segments of the Protein
                obj1 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P3");

                //Change their material
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();

                //Required step
                if (exitState[4] >= 3)
                    next();
                break;

            //2 K+ goes through the Na K+ ATPase Protein into the Cell
            case 2:
                exitState = pipes[1].exitState;
                whiteMats[0].SetActive(false);
                whiteMats[1].SetActive(true);

                // The Na K+ ATPase is bidirectional so the previous pipe is disabled to prevent overlap
                pipeObjs[1].SetActive(true);
                pipeObjs[0].SetActive(false);
                obj1 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P3");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();
                if (exitState[2] >= 2)
                    next();
                break;

            //3 Na+, 3K+, 6 Cl- go through the Na K+ 2Cl- cotransport into the cell
            case 3:
                whiteMats[2].SetActive(true);
                whiteMats[1].SetActive(false);
                exitState = pipes[2].exitState;
                obj1 = pipeObjs[2].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[2].transform.parent.transform.parent.transform.Find("P2");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                if (exitState[4] >= 3 && exitState[2] >= 3 && exitState[1] >= 6)
                    next();
                break;

            // The 3 K+ from the previous step exits the cell through the K+ Channel
            case 4:
                whiteMats[2].SetActive(false);
                whiteMats[3].SetActive(true);
                exitState = pipes[3].exitState;
                obj1 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P3");
                obj4 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P4");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();
                obj4.GetComponent<ProteinBehavior>().highlight();
                if (exitState[3] >= 3)
                    next();
                break;

            // Another 3 Na+ goes through the Na K+ ATPase Protein to the extracellular space
            case 5:
                whiteMats[3].SetActive(false);
                whiteMats[0].SetActive(true);
                pipeObjs[0].SetActive(true);
                pipeObjs[1].SetActive(false);
                exitState = pipes[0].exitState;
                obj1 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[0].transform.parent.transform.parent.transform.Find("P3");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();
                if (exitState[4] >= 6)
                    next();
                break;

            // Another 2 K+ goes through the Na K+ ATPase Protein into the Cell
            case 6:
                exitState = pipes[1].exitState;
                whiteMats[0].SetActive(false);
                whiteMats[1].SetActive(true);
                pipeObjs[1].SetActive(true);
                pipeObjs[0].SetActive(false);
                obj1 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[1].transform.parent.transform.parent.transform.Find("P3");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();
                if (exitState[2] >= 4)
                    next();
                break;

            // The 4 K+ ions from the NA K+ ATPase go through the K+ Channel
            case 7:
                whiteMats[1].SetActive(false);
                whiteMats[3].SetActive(true);
                exitState = pipes[3].exitState;
                obj1 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P2");
                obj3 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P3");
                obj4 = pipeObjs[3].transform.parent.transform.parent.transform.Find("P4");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();
                obj3.GetComponent<ProteinBehavior>().highlight();
                obj4.GetComponent<ProteinBehavior>().highlight();
                if (exitState[2] >= 4)
                    next();
                break;

            // The 6 Cl- ions exit the cell through the Cl- Channel
            case 8:
                whiteMats[3].SetActive(false);
                whiteMats[4].SetActive(true);
                exitState = pipes[4].exitState;
                obj1 = pipeObjs[4].transform.parent.transform.parent.transform.Find("P1");
                obj2 = pipeObjs[4].transform.parent.transform.parent.transform.Find("P2");
                obj1.GetComponent<ProteinBehavior>().highlight();
                obj2.GetComponent<ProteinBehavior>().highlight();

                //Starts the quiz
                if (exitState[1] >= 6)
                    startQuiz();
                break;

            //Only occurs during the quiz phase
            default:
                break;



        }

    }

    // Starts the next coroutine if it is not currently running
    public void next()
    {
        if (!isNextRunning)
        {
            StartCoroutine("_next");
        }
    }

    //Waits half a second then increments the timer to move to the next step
    IEnumerator _next()
    {
        isNextRunning = true;
        Debug.Log("Started");
        yield return new WaitForSeconds(0.5f);
        Debug.Log("Timer Done");
        nextButton.SetActive(false);
        ++counter;

        // Returns all proteins and pipes to their default color 
        foreach (GameObject pipe in pipeObjs)
        {
            // Only occurs on default pipes but works the same
            if (pipe.transform.parent.transform.parent == null)
            {
                var obj = pipe.transform.parent.transform.Find("Cylinder");
                obj.GetComponent<MeshRenderer>().material = defMat;
            }
            else
            {
                // Every protein has at least two segments then checks if any more exist
                var obj1 = pipe.transform.parent.transform.parent.transform.Find("P1");
                var obj2 = pipe.transform.parent.transform.parent.transform.Find("P2");
                obj1.GetComponent<ProteinBehavior>().lowlight();
                obj2.GetComponent<ProteinBehavior>().lowlight();
                if (pipe.transform.parent.transform.parent.transform.Find("P3") != null)
                {
                    var obj3 = pipe.transform.parent.transform.parent.transform.Find("P3");
                    obj3.GetComponent<ProteinBehavior>().lowlight();
                    if (pipe.transform.parent.transform.parent.transform.Find("P4") != null)
                    {
                        var obj4 = pipe.transform.parent.transform.parent.transform.Find("P4");
                        obj4.GetComponent<ProteinBehavior>().lowlight();
                    }
                }
            }
        }
        // Update the current directions on the board
        directions.text = taskText[counter];
        isNextRunning = false;
    }

    //Moves to the next scene, however this is the last scene
    public void nextScene()
    {
        Debug.Log("Done");
        //SceneChange.clearObjects();
        //SceneManager.LoadScene("Cell 2");
    }

    // Starts the quiz phase
    private void startQuiz()
    {
        // Clear the directions and activate the panels needed for the quiz
        directions.text = "";
        quizPanel.SetActive(true);
        ansPanel.SetActive(true);
        counter++;
    }
}
